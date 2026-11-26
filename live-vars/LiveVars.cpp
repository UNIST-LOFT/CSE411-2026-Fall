#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Plugins/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace
{
  using ValueSet = SmallPtrSet<const Value *, 32>;

  bool tracked(const Value *V)
  {
    return isa<Argument>(V) || (isa<Instruction>(V) && !V->getType()->isVoidTy());
  }

  void printLiveVars(const Function &F,
                     const DenseMap<const BasicBlock *, ValueSet> &In,
                     const DenseMap<const BasicBlock *, ValueSet> &Out)
  {
    SmallVector<const Value *, 32> Values;
    for (const Argument &Arg : F.args())
      Values.push_back(&Arg);

    for (const BasicBlock &B : F)
      for (const Instruction &I : B)
        if (tracked(&I))
          Values.push_back(&I);

    auto PrintSet = [&](const ValueSet &Set)
    {
      errs() << "{";
      bool First = true;
      for (const Value *V : Values)
      {
        if (!Set.contains(V))
          continue;
        if (!First)
          errs() << ", ";
        V->printAsOperand(errs(), false);
        First = false;
      }
      errs() << "}";
    };
    errs() << "function ";
    F.printAsOperand(errs(), false);
    errs() << "\n";
    for (const BasicBlock &B : F)
    {
      errs() << "  block ";
      B.printAsOperand(errs(), false);
      errs() << "\n    in:  ";
      PrintSet(In.find(&B)->second);
      errs() << "\n    out: ";
      PrintSet(Out.find(&B)->second);
      errs() << "\n";
    }
  }

  class LiveVarsPass : public PassInfoMixin<LiveVarsPass>
  {
  public:
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &)
    {
      DenseMap<const BasicBlock *, ValueSet> In, Out;
      bool Changed;
      do
      {
        Changed = false;
        for (const BasicBlock *const B : post_order(&F.getEntryBlock()))
        {
          ValueSet NewOut;
          for (const BasicBlock *const Succ : successors(B))
          {
            NewOut.insert(In[Succ].begin(), In[Succ].end());

            for (const PHINode &Phi : Succ->phis())
              for (unsigned i = 0; i < Phi.getNumIncomingValues(); ++i)
                if (Phi.getIncomingBlock(i) == B && tracked(Phi.getIncomingValue(i)))
                  NewOut.insert(Phi.getIncomingValue(i));
          }
          ValueSet NewIn = NewOut;
          for (auto i = B->rbegin(); i != B->rend(); ++i)
          {
            const Instruction &Inst = *i;
            NewIn.erase(&Inst);
            if (!isa<PHINode>(Inst))
              for (const Use &Operand : Inst.operands())
                if (tracked(Operand.get()))
                  NewIn.insert(Operand.get());
          }
          if (NewIn != In[B] || NewOut != Out[B])
          {
            In[B] = std::move(NewIn);
            Out[B] = std::move(NewOut);
            Changed = true;
          }
        }
      } while (Changed);

      printLiveVars(F, In, Out);
      return PreservedAnalyses::all();
    }
  };
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo()
{
  return {LLVM_PLUGIN_API_VERSION,
          "LiveVars",
          LLVM_VERSION_STRING,
          [](PassBuilder &PB)
          {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>)
                {
                  if (Name != "live-vars")
                    return false;
                  FPM.addPass(LiveVarsPass());
                  return true;
                });
          }};
}
