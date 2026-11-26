import pathlib
import re
import subprocess
import sys
import tempfile

opt, lli, plugin, directory = sys.argv[1:]
source = pathlib.Path(directory) / "cases.ll"

with tempfile.TemporaryDirectory() as tmp:
    output = pathlib.Path(tmp) / "optimized.ll"
    subprocess.run([opt, "-load-pass-plugin", plugin, "-passes=constant-prop",
                    "-verify-each", "-S", str(source), "-o", str(output)],
                   check=True)
    ir = output.read_text()

    def function(name):
        match = re.search(r"define\s+[^@]*@" + name + r"\([^\n]*\)[^{]*\{(.*?)^\}",
                          ir, re.MULTILINE | re.DOTALL)
        assert match, f"missing function {name}"
        return match.group(1)

    straight = function("straight")
    assert "ret i32 24" in straight, straight
    assert not re.search(r"\b(alloca|load|store|add|mul)\b", straight), straight
    branch = function("branch")
    assert "ret i32 42" in branch, branch
    assert "ret i32 99" not in branch and "br i1" not in branch, branch
    unknown = function("unknown")
    assert "%condition" in unknown and "%x" in unknown, unknown
    assert "ret i32 18" in function("backwards_order")
    assert "ret i32 6" in function("same_phi")
    assert "ret i32 9" in function("edge_constants")
    assert "phi i32" in function("different_edges")
    # A poison-producing overflow may be refined to a concrete value.
    assert re.search(r"ret i32 (?:poison|-2147483648)", function("overflow"))
    loop = function("loop")
    assert "%n" in loop and "add i32" in loop, loop
    memory = function("memory")
    assert "call void @write_cell()" in memory and "load i32, ptr @cell" in memory, memory
    escaping = function("escaping_slot")
    assert "alloca i32" in escaping and "load i32, ptr %slot" in escaping, escaping

    # Both versions must run correctly, including both unknown branch outcomes,
    # a changing loop value, and calls that modify memory.
    for path in (source, output):
        subprocess.run([lli, str(path)], check=True)
