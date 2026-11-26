cmake_minimum_required(VERSION 3.20)
project(LiveVars LANGUAGES C CXX)

find_package(LLVM REQUIRED CONFIG)
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)
add_llvm_pass_plugin(LiveVars LiveVars.cpp)
target_include_directories(LiveVars PRIVATE ${LLVM_INCLUDE_DIRS})
target_compile_definitions(LiveVars PRIVATE ${LLVM_DEFINITIONS})
target_compile_features(LiveVars PRIVATE cxx_std_17)

enable_testing()
find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_test(NAME live-vars
  COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_live_vars.py"
          "${LLVM_TOOLS_BINARY_DIR}/opt" "$<TARGET_FILE:LiveVars>"
          "${CMAKE_CURRENT_SOURCE_DIR}/tests")
