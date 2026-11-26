#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Plugins/PassPlugin.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/PromoteMemToReg.h"

using namespace llvm;

namespace
{
  using Constants = DenseMap<const Value *, Constant *>;
  struct State
  {
    explicit State(bool Top = false) : IsTop(Top), Facts() {}
    bool IsTop = false;
    Constants Facts;
  };
  using BlockConstants = DenseMap<const BasicBlock *, State>;

  Constant *lookup(const Value *V, const State &S)
  {
    if (auto *C = dyn_cast<Constant>(V))
      return const_cast<Constant *>(C);
    return S.Facts.lookup(V);
  }

  // Top is the identity for meet; concrete states contain known constants.
  State meet(State Left, const State &Right)
  {
    if (Left.IsTop)
      return Right;
    if (Right.IsTop)
      return Left;
    for (auto It = Left.Facts.begin(); It != Left.Facts.end();)
    {
      auto Current = It++;
      if (Right.Facts.lookup(Current->first) != Current->second)
        Left.Facts.erase(Current);
    }
    return Left;
  }

  Constant *constantPhi(const PHINode &Phi, const BlockConstants &Out)
  {
    Constant *Result = nullptr;
    for (unsigned I = 0; I < Phi.getNumIncomingValues(); ++I)
    {
      Constant *C = lookup(Phi.getIncomingValue(I),
                           Out.find(Phi.getIncomingBlock(I))->second);
      if (!C || (Result && Result != C))
        return nullptr;
      Result = C;
    }
    return Result;
  }

  Constant *evaluate(Instruction &I, const State &S,
                     const BlockConstants &Out, const DataLayout &DL)
  {
    if (I.isTerminator() || I.getType()->isVoidTy() ||
        I.mayHaveSideEffects() || isa<LoadInst>(I))
      return nullptr;
    if (auto *Phi = dyn_cast<PHINode>(&I))
      return constantPhi(*Phi, Out);

    SmallVector<Constant *, 8> Operands;
    for (const Use &Operand : I.operands())
    {
      Constant *C = lookup(Operand.get(), S);
      if (!C)
        return nullptr;
      Operands.push_back(C);
    }
    return ConstantFoldInstOperands(&I, Operands, DL, nullptr, false);
  }

  State transfer(BasicBlock &B, State S, const BlockConstants &Out,
                 const DataLayout &DL)
  {
    for (Instruction &I : B)
    {
      S.Facts.erase(&I);
      if (Constant *C = evaluate(I, S, Out, DL))
        S.Facts[&I] = C;
    }
    return S;
  }

  bool promoteLocals(Function &F)
  {
    SmallVector<AllocaInst *, 16> Allocas;
    for (Instruction &I : F.getEntryBlock())
      if (auto *AI = dyn_cast<AllocaInst>(&I); AI && isAllocaPromotable(AI))
        for (User *U : AI->users())
          if (isa<LoadInst>(U))
          {
            Allocas.push_back(AI);
            break;
          }
    if (Allocas.empty())
      return false;

    DominatorTree DT(F);
    PromoteMemToReg(Allocas, DT);
    return true;
  }

  bool applyConstants(Function &F, const BlockConstants &Out)
  {
    bool Changed = false;
    for (BasicBlock &B : F)
      for (auto It = B.begin(); It != B.end();)
      {
        Instruction &I = *It++;
        Constant *C = Out.find(&B)->second.Facts.lookup(&I);
        if (!C)
          continue;
        I.replaceAllUsesWith(C);
        I.eraseFromParent();
        Changed = true;
      }
    for (BasicBlock &B : F)
      Changed |= ConstantFoldTerminator(&B);
    return Changed;
  }

  class ConstantPropPass : public PassInfoMixin<ConstantPropPass>
  {
  public:
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &)
    {
      bool Changed = promoteLocals(F);
      const DataLayout &DL = F.getParent()->getDataLayout();
      bool LocalChange;
      do
      {
        LocalChange = removeUnreachableBlocks(F);

        // Compute block inputs and outputs without changing the IR. Each
        // successor receives the facts produced by its predecessors.
        BlockConstants In, Out;
        for (BasicBlock &B : F)
        {
          In[&B];
          Out[&B];
        }
        const State Top(true);
        bool FactsChanged;
        do
        {
          FactsChanged = false;
          for (BasicBlock &B : F)
          {
            State NewIn = Top;
            for (BasicBlock *P : predecessors(&B))
              NewIn = meet(std::move(NewIn), Out[P]);

            // The entry block has no incoming facts.
            if (NewIn.IsTop)
              NewIn = State();
            State NewOut = transfer(B, NewIn, Out, DL);
            if (NewIn.Facts != In[&B].Facts || NewOut.Facts != Out[&B].Facts)
            {
              In[&B] = std::move(NewIn);
              Out[&B] = std::move(NewOut);
              FactsChanged = true;
            }
          }
        } while (FactsChanged);

        LocalChange |= applyConstants(F, Out);
        Changed |= LocalChange;
      } while (LocalChange);

      return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
    }
  };
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo()
{
  return {LLVM_PLUGIN_API_VERSION,
          "ConstantProp",
          LLVM_VERSION_STRING,
          [](PassBuilder &PB)
          {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement> InnerPipeline)
                {
                  if (Name != "constant-prop" || !InnerPipeline.empty())
                    return false;
                  FPM.addPass(ConstantPropPass());
                  return true;
                });
          }};
}
