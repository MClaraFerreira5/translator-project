file(REMOVE_RECURSE "${DESTINO}")
execute_process(COMMAND "${CMAKE_COMMAND}" -S "${FONTE}" -B "${DESTINO}"
  -G "${GERADOR}" -DBUILD_TESTING=OFF "-DCMAKE_CXX_COMPILER=${COMPILADOR}"
  "-DFLEX_EXECUTABLE=${FLEX_EXE}" "-DBISON_EXECUTABLE=${BISON_EXE}"
  RESULT_VARIABLE resultado)
if(NOT resultado EQUAL 0)
  message(FATAL_ERROR "Configuração limpa falhou: ${resultado}")
endif()
execute_process(COMMAND "${CMAKE_COMMAND}" --build "${DESTINO}" RESULT_VARIABLE resultado)
if(NOT resultado EQUAL 0)
  message(FATAL_ERROR "Build limpo falhou: ${resultado}")
endif()
execute_process(COMMAND "${DESTINO}/oxente" --help RESULT_VARIABLE resultado OUTPUT_VARIABLE ajuda)
if(NOT resultado EQUAL 0 OR NOT ajuda MATCHES "Uso:")
  message(FATAL_ERROR "Executável do build limpo não funciona")
endif()
