# Assignment 10: Live Variable Analysis

This module provides a new-pass-manager LLVM function pass, `live-vars`, in [LiveVars.cpp](LiveVars.cpp). It computes live-in and live-out sets for every basic block of each defined function, using LLVM IR produced by `sjc-ll-gen`.

## Prerequisites

Install LLVM 23.1.0 (including `llvm-config`, `opt`, and development headers), CMake 3.20 or later, a C++17 compiler, Python 3. The pass must be built against the same LLVM installation and architecture as the `opt` used to load it.

## Build and test

From the repository root:

```sh
cmake -S live-vars -B live-vars/target/build -DLLVM_DIR="$(llvm-config --cmakedir)"
cmake --build live-vars/target/build
ctest --test-dir live-vars/target/build --output-on-failure
```

## What to Submit

Write a document that answers the following two questions.

#### Q1. How do the analysis results for [cases.ll](./tests/cases.ll) align with your understanding of live variable analysis?

Specifically, explain the dataflow equations for live variable analysis and relate them to the analysis results.

#### Q2. How does [LiveVars.cpp](./LiveVars.cpp) implement the dataflow equations for live variable analysis?

Explain how each relevant segment of the source code implements the dataflow equations for live variable analysis.

### How to Submit

Submit the following file via BlackBoard.

* A document in pdf format

### Submission Deadline

**Due: December 1, 2026; 3:00pm KST**
