package sjc.livevars;

import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Paths;
import org.eclipse.jdt.core.dom.CompilationUnit;
import sjc.ast.ASTUtil;
import sjc.llgen.LLVMIRGenerator;
import sjc.symboltable.SymbolTable;
import sjc.symboltable.SymbolTableBuilder;
import sjc.type.TypeFactory;
import sjc.type.checker.TypeChecker;
import sjc.type.checker.TypeTable;

/** Generates the input to the live-vars LLVM pass from a Static Java file. */
public final class EmitLLVM {
  public static void main(final String[] args) throws Exception {
    if (args.length != 2) {
      throw new IllegalArgumentException("Usage: EmitLLVM <source.java> <output.ll>");
    }
    final CompilationUnit ast = ASTUtil.ast(args[0]);
    final SymbolTable symbols = SymbolTableBuilder.build(ast);
    final TypeTable types = TypeChecker.check(new TypeFactory(), ast, symbols);
    final String ir = LLVMIRGenerator.generate(ast, symbols, types);
    Files.write(Paths.get(args[1]), ir.getBytes(StandardCharsets.UTF_8));
  }
}
