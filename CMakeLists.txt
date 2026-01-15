cmake_minimum_required(VERSION 3.31)

project(wyrm-lang LANGUAGES C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

add_executable(wyrm
	src/main.c
	src/utils/memory.c
	src/vm/chunk.c
	src/vm/debug.c
	src/vm/value.c
	src/vm/vm.c
	src/frontend/lexer/lexer.c
	src/frontend/parser/parser.c
	src/frontend/parser/expr.c
)
target_include_directories(wyrm PUBLIC "${CMAKE_SOURCE_DIR}/src")