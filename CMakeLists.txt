cmake_minimum_required(VERSION 3.16.3)

project(my_compiler)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})
set(CMAKE_CXX_STANDARD 17)

#add_compile_options(-g -DDEBUG)
add_compile_options(-fno-rtti)

# LLVM
set(LLVM_DIR /home/ihopenot/llvm/lib/cmake/llvm)

find_package(LLVM)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")

include_directories(${LLVM_INCLUDE_DIR})
message(STATUS "LLVM include path ${LLVM_INCLUDE_DIR}")
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs support core irreader native)

# Flex
find_package(FLEX)
FLEX_TARGET(lex ${PROJECT_SOURCE_DIR}/lex.l ${PROJECT_BINARY_DIR}/lex.cpp
            DEFINES_FILE ${PROJECT_SOURCE_DIR}/lex.h)

# Bison
find_package(BISON)
BISON_TARGET(grammar ${PROJECT_SOURCE_DIR}/grammar.y ${PROJECT_BINARY_DIR}/grammar.cpp
            DEFINES_FILE ${PROJECT_SOURCE_DIR}/grammar.hpp )

ADD_FLEX_BISON_DEPENDENCY(lex grammar)

# resolve library dependency
add_library(myc_grammar ${FLEX_lex_OUTPUTS} ${BISON_grammar_OUTPUTS})
add_library(myc_lib ASTnode.cpp codegen.cpp typefactor.cpp objgen.cpp)

add_executable(compiler ${PROJECT_SOURCE_DIR}/main.cpp)
target_link_libraries(compiler myc_grammar myc_lib ${llvm_libs})

add_executable(runtest ${PROJECT_SOURCE_DIR}/test/test.cpp)

add_custom_target(test ${PROJECT_BINARY_DIR}/runtest ${PROJECT_SOURCE_DIR}/test/)