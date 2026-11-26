cmake_minimum_required(VERSION 3.20)
project(ConstantProp LANGUAGES C CXX)

find_package(LLVM REQUIRED CONFIG)
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)
add_llvm_pass_plugin(ConstantProp ConstantProp.cpp)
target_include_directories(ConstantProp PRIVATE ${LLVM_INCLUDE_DIRS})
target_compile_definitions(ConstantProp PRIVATE ${LLVM_DEFINITIONS})
target_compile_features(ConstantProp PRIVATE cxx_std_17)

enable_testing()
find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_test(NAME constant-prop
  COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_constant_prop.py"
          "${LLVM_TOOLS_BINARY_DIR}/opt" "${LLVM_TOOLS_BINARY_DIR}/lli"
          "$<TARGET_FILE:ConstantProp>" "${CMAKE_CURRENT_SOURCE_DIR}/tests")
