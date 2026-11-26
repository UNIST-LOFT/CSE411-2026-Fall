# Assignment 11: Constant Propagation

This module provides a new-pass-manager LLVM function pass, `constant-prop`, in [ConstantProp.cpp](ConstantProp.cpp). It propagates known constant values through each defined function and simplifies instructions and branches where possible.

## Prerequisites

Install LLVM 23.1.0 (including `llvm-config`, `opt`, `lli`, and development headers), CMake 3.20 or later, a C++17 compiler, and Python 3. The pass must be built against the same LLVM installation and architecture as the `opt` used to load it.

## Build and test

From the repository root:

```sh
cmake -S constant-prop -B constant-prop/target/build -DLLVM_DIR="$(llvm-config --cmakedir)"
cmake --build constant-prop/target/build
ctest --test-dir constant-prop/target/build --output-on-failure
```

## What to Submit

Write a document that answers the following two questions.

#### Q1. How do the analysis results for [cases.ll](./tests/cases.ll) align with your understanding of constant propagation?

Specifically, explain the dataflow equations for constant propagation and relate them to the analysis results. 

#### Q2. How does [ConstantProp.cpp](./ConstantProp.cpp) implement the dataflow equations for constant propagation?

Explain how the source code implements the dataflow equations and uses the analysis results to perform constant propagation.

### How to Submit

Submit the following file via BlackBoard.

* A document in pdf format

### Submission Deadline

**Due: December 3, 2026; 3:00pm KST**
