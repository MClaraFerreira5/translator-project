cmake_minimum_required(VERSION 3.16)
project(Oxente LANGUAGES CXX)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "Use uma árvore separada: cmake -S . -B build")
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# REQUIRED informa a ferramenta ausente; no Fedora: dnf install flex bison.
find_package(FLEX 2.6 REQUIRED)
find_package(BISON 3.0 REQUIRED)
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated")
BISON_TARGET(InfraParser src/frontend/infra.y
  "${CMAKE_CURRENT_BINARY_DIR}/generated/infra.tab.cpp"
  DEFINES_FILE "${CMAKE_CURRENT_BINARY_DIR}/generated/infra.tab.h")
FLEX_TARGET(InfraLexer src/frontend/infra.l
  "${CMAKE_CURRENT_BINARY_DIR}/generated/infra.lex.cpp")
ADD_FLEX_BISON_DEPENDENCY(InfraLexer InfraParser)
add_library(oxente_frontend STATIC src/frontend/scanner.cpp
  ${BISON_InfraParser_OUTPUTS} ${FLEX_InfraLexer_OUTPUTS})
target_include_directories(oxente_frontend PUBLIC
  "${CMAKE_CURRENT_BINARY_DIR}/generated" "${CMAKE_CURRENT_SOURCE_DIR}/src/frontend")
add_executable(oxente src/main.cpp)
target_link_libraries(oxente PRIVATE oxente_frontend)
add_executable(oxente-tokens src/frontend/tokens_main.cpp)
target_link_libraries(oxente-tokens PRIVATE oxente_frontend)
include(CTest)
if(BUILD_TESTING)
  add_executable(teste-lexer tests/lexer.cpp)
  target_link_libraries(teste-lexer PRIVATE oxente_frontend)
  add_test(NAME lexer COMMAND teste-lexer)
  add_test(NAME lexer_harness COMMAND "${CMAKE_COMMAND}"
    -DTOKENS=$<TARGET_FILE:oxente-tokens>
    -DDESTINO=${CMAKE_CURRENT_BINARY_DIR}/teste-harness
    -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/lexer-harness.cmake")
  add_test(NAME cli COMMAND "${CMAKE_COMMAND}"
    -DOXENTE=$<TARGET_FILE:oxente> -DFONTE=${CMAKE_CURRENT_SOURCE_DIR}
    -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/cli.cmake")
  add_test(NAME build_limpo COMMAND "${CMAKE_COMMAND}"
    -DFONTE=${CMAKE_CURRENT_SOURCE_DIR}
    -DDESTINO=${CMAKE_CURRENT_BINARY_DIR}/teste-build-limpo
    -DGERADOR=${CMAKE_GENERATOR} -DCOMPILADOR=${CMAKE_CXX_COMPILER}
    -DFLEX_EXE=${FLEX_EXECUTABLE} -DBISON_EXE=${BISON_EXECUTABLE}
    -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/build-limpo.cmake")
  add_executable(teste-diagnosticos tests/teste_diagnosticos.cpp)
  target_include_directories(teste-diagnosticos PRIVATE src/common)
  add_test(NAME diagnosticos COMMAND teste-diagnosticos)
endif()
