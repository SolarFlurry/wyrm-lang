cmake_minimum_required(VERSION 3.31)

project(wyrm-lang LANGUAGES C CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

add_executable(wyrm
	src/main.c
	src/utils/memory.c
	src/vm/chunk.c
	src/vm/debug.c
	src/vm/value.c
	src/vm/vm.c
	src/compiler/lexer/lexer.c
	src/compiler/parser/parser.c
	src/compiler/parser/expr.c
	src/compiler/parser/stmt.c
	src/compiler/parser/type.c
	src/compiler/error/error.c
	src/compiler/ast.c
	src/compiler/codegen/codegen.c
	src/compiler/compiler.c
	src/compiler/token.c
	src/compiler/semantic_analysis/symtable.c
	src/compiler/semantic_analysis/name_res.c
	src/compiler/semantic_analysis/typechecker.c
	src/compiler/semantic_analysis/expr.c
)
target_include_directories(wyrm PUBLIC "${CMAKE_SOURCE_DIR}/src")

add_executable(test
	src/test.cpp
)
target_compile_features(test PRIVATE cxx_std_23)