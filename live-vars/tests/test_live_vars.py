import pathlib
import subprocess
import sys

opt, plugin, directory = sys.argv[1:]
source = pathlib.Path(directory) / "cases.ll"
result = subprocess.run([opt, "-load-pass-plugin", plugin, "-passes=live-vars",
                         "-disable-output", str(source)],
                        check=True, capture_output=True, text=True)
expected = '''function @diamond
  block %entry
    in:  {%cond, %a, %b}
    out: {%a, %b}
  block %left
    in:  {%a}
    out: {%a}
  block %right
    in:  {%b}
    out: {%b}
  block %merge
    in:  {}
    out: {}
function @loop
  block %entry
    in:  {%n}
    out: {%n}
  block %loop
    in:  {%n}
    out: {%n, %i, %next}
  block %exit
    in:  {%i}
    out: {}
function @memory
  block %entry
    in:  {%x}
    out: {}
function @unnamed
  block %1
    in:  {%0}
    out: {}
'''
if result.stderr != expected:
    import difflib
    sys.stderr.writelines(difflib.unified_diff(
        expected.splitlines(True), result.stderr.splitlines(True),
        fromfile="expected", tofile="actual"))
    sys.exit(1)
