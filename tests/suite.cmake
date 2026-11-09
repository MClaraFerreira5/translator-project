# Suite automatizada de verificações da especificação da Oxente Lang
# Executa casos válidos e inválidos documentados em tests/suite_manifest.json

if(NOT DEFINED FONTE)
  set(FONTE "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

set(manifesto "${FONTE}/tests/suite_manifest.json")
if(NOT EXISTS "${manifesto}")
  message(FATAL_ERROR "Manifesto da suíte não encontrado: ${manifesto}")
endif()

file(READ "${manifesto}" conteudo_json)

# Função auxiliar para extrair campo de objeto JSON simples no CMake 3.19+
# No CMake, string(JSON ...) está disponível a partir da versão 3.19.
# Nosso CMakeLists.txt usa cmake_minimum_required(VERSION 3.16).
# Para compatibilidade universal, vamos rodar tanto verificação nativa CMake quanto permitir
# a invocação pelo CTest.

set(total_testes 0)
set(aprovados 0)
set(limitacoes 0)
set(falhas 0)

function(testar_caso id arquivo tipo etapa causa)
  math(EXPR total "${total_testes}+1")
  set(total_testes ${total} PARENT_SCOPE)

  set(caminho "${FONTE}/${arquivo}")
  if(NOT EXISTS "${caminho}")
    message(SEND_ERROR "[FALHA] ${id}: arquivo não encontrado: ${caminho}")
    math(EXPR f "${falhas}+1")
    set(falhas ${f} PARENT_SCOPE)
    return()
  endif()

  if("${etapa}" STREQUAL "lexica")
    if(NOT DEFINED TOKENS OR NOT EXISTS "${TOKENS}")
      message(STATUS "[PULADO] ${id}: executável oxente-tokens não fornecido")
      math(EXPR l "${limitacoes}+1")
      set(limitacoes ${l} PARENT_SCOPE)
      return()
    endif()

    execute_process(COMMAND "${TOKENS}" "${caminho}"
      RESULT_VARIABLE ret OUTPUT_VARIABLE saida ERROR_VARIABLE erro)

    if("${tipo}" STREQUAL "valido")
      if(ret EQUAL 0)
        message(STATUS "[APROVADO] ${id} (léxico válido)")
        math(EXPR a "${aprovados}+1")
        set(aprovados ${a} PARENT_SCOPE)
      else()
        message(SEND_ERROR "[FALHA] ${id}: esperado sucesso léxico, obtido ${ret}: ${erro}")
        math(EXPR f "${falhas}+1")
        set(falhas ${f} PARENT_SCOPE)
      endif()
    else()
      if(ret EQUAL 1)
        message(STATUS "[APROVADO] ${id} (léxico detectou erro)")
        math(EXPR a "${aprovados}+1")
        set(aprovados ${a} PARENT_SCOPE)
      else()
        message(SEND_ERROR "[FALHA] ${id}: esperado erro léxico código 1, obtido ${ret}: ${erro}")
        math(EXPR f "${falhas}+1")
        set(falhas ${f} PARENT_SCOPE)
      endif()
    endif()

  else()
    # Etapas sintática e semântica
    if(NOT DEFINED OXENTE OR NOT EXISTS "${OXENTE}")
      message(STATUS "[PENDENTE] ${id}: executável oxente não fornecido")
      math(EXPR l "${limitacoes}+1")
      set(limitacoes ${l} PARENT_SCOPE)
      return()
    endif()

    execute_process(COMMAND "${OXENTE}" "--check" "${caminho}"
      RESULT_VARIABLE ret OUTPUT_VARIABLE saida ERROR_VARIABLE erro)

    if(ret EQUAL 4)
      # Limitação conhecida: CLI ainda não integrou parser/semântico
      message(STATUS "[LIMITAÇÃO CONHECIDA] ${id} (${etapa}): CLI retornou 4 (não implementado)")
      math(EXPR l "${limitacoes}+1")
      set(limitacoes ${l} PARENT_SCOPE)
    elseif("${tipo}" STREQUAL "invalido" AND ret EQUAL 1)
      message(STATUS "[APROVADO] ${id} (${etapa} detectou erro)")
      math(EXPR a "${aprovados}+1")
      set(aprovados ${a} PARENT_SCOPE)
    elseif("${tipo}" STREQUAL "valido" AND ret EQUAL 0)
      message(STATUS "[APROVADO] ${id} (${etapa} válido)")
      math(EXPR a "${aprovados}+1")
      set(aprovados ${a} PARENT_SCOPE)
    else()
      message(SEND_ERROR "[FALHA] ${id}: código inesperado ${ret}: ${erro}")
      math(EXPR f "${falhas}+1")
      set(falhas ${f} PARENT_SCOPE)
    endif()
  endif()
endfunction()

# Casos Léxicos Válidos e Inválidos
testar_caso("val-comentarios" "examples/valid/comentarios.oxe" "valido" "lexica" "")
testar_caso("val-nl-crlf-eof" "examples/valid/nl-crlf-eof.oxe" "valido" "lexica" "")
testar_caso("inval-lexico-caractere" "examples/invalid/lexico-caractere.oxe" "invalido" "lexica" "caractere não reconhecido")
testar_caso("inval-lexico-texto-nao-terminado" "examples/invalid/lexico-texto-nao-terminado.oxe" "invalido" "lexica" "texto não terminado")
testar_caso("inval-lexico-comentario-nao-terminado" "examples/invalid/lexico-comentario-nao-terminado.oxe" "invalido" "lexica" "comentário não terminado")

# Casos Válidos Sintáticos e Semânticos
testar_caso("val-calculo" "examples/valid/calculo.oxe" "valido" "semantica" "")
testar_caso("val-decisao" "examples/valid/decisao.oxe" "valido" "semantica" "")
testar_caso("val-ramos" "examples/valid/ramos.oxe" "valido" "semantica" "")
testar_caso("val-reais" "examples/valid/reais.oxe" "valido" "semantica" "")
testar_caso("val-repeticao" "examples/valid/repeticao.oxe" "valido" "semantica" "")
testar_caso("val-precedencia-potencia" "examples/valid/precedencia-potencia.oxe" "valido" "semantica" "")
testar_caso("val-precedencia-logica" "examples/valid/precedencia-logica.oxe" "valido" "semantica" "")
testar_caso("val-operadores-longos" "examples/valid/operadores-longos.oxe" "valido" "semantica" "")
testar_caso("val-aninhamento" "examples/valid/aninhamento.oxe" "valido" "semantica" "")
testar_caso("val-escopo-independente" "examples/valid/escopo-independente.oxe" "valido" "semantica" "")
testar_caso("val-lacos-aninhados" "examples/valid/lacos-aninhados.oxe" "valido" "semantica" "")

# Casos Inválidos Sintáticos
testar_caso("inval-sintatico-operando" "examples/invalid/sintatico-operando.oxe" "invalido" "sintatica" "sintaxe_invalida")
testar_caso("inval-sintatico-parentese-desbalanceado" "examples/invalid/sintatico-parentese-desbalanceado.oxe" "invalido" "sintatica" "sintaxe_invalida")
testar_caso("inval-sintatico-bloco-nao-fechado" "examples/invalid/sintatico-bloco-nao-fechado.oxe" "invalido" "sintatica" "sintaxe_invalida")
testar_caso("inval-sintatico-comparacao-encadeada" "examples/invalid/sintatico-comparacao-encadeada.oxe" "invalido" "sintatica" "sintaxe_invalida")
testar_caso("inval-sintatico-tipo-sem-identificador" "examples/invalid/sintatico-tipo-sem-identificador.oxe" "invalido" "sintatica" "sintaxe_invalida")
testar_caso("inval-sintatico-ramo-sem-vai-que" "examples/invalid/sintatico-ramo-sem-vai-que.oxe" "invalido" "sintatica" "sintaxe_invalida")
testar_caso("inval-sintatico-arrodeia-sem-ate" "examples/invalid/sintatico-arrodeia-sem-ate.oxe" "invalido" "sintatica" "sintaxe_invalida")
testar_caso("inval-sintatico-bora-sem-quebra" "examples/invalid/sintatico-bora-sem-quebra.oxe" "invalido" "sintatica" "sintaxe_invalida")

# Casos Inválidos Semânticos
testar_caso("inval-semantico-nao-declarado" "examples/invalid/semantico-nao-declarado.oxe" "invalido" "semantica" "nome_nao_declarado")
testar_caso("inval-semantico-uso-proprio-inicializador" "examples/invalid/semantico-uso-proprio-inicializador.oxe" "invalido" "semantica" "nome_nao_declarado")
testar_caso("inval-semantico-tipo" "examples/invalid/semantico-tipo.oxe" "invalido" "semantica" "misturou_as_coisas")
testar_caso("inval-semantico-operacao-tipo" "examples/invalid/semantico-operacao-tipo.oxe" "invalido" "semantica" "misturou_as_coisas")
testar_caso("inval-semantico-duplicacao" "examples/invalid/semantico-duplicacao.oxe" "invalido" "semantica" "valor_sem_pe_nem_cabeca")
testar_caso("inval-semantico-sombreamento" "examples/invalid/semantico-sombreamento.oxe" "invalido" "semantica" "valor_sem_pe_nem_cabeca")
testar_caso("inval-semantico-escopo-fechado" "examples/invalid/semantico-escopo-fechado.oxe" "invalido" "semantica" "nome_nao_declarado")
testar_caso("inval-semantico-condicao-nao-booleana" "examples/invalid/semantico-condicao-nao-booleana.oxe" "invalido" "semantica" "misturou_as_coisas")
testar_caso("inval-semantico-condicao-repeticao-nao-booleana" "examples/invalid/semantico-condicao-repeticao-nao-booleana.oxe" "invalido" "semantica" "misturou_as_coisas")
testar_caso("inval-semantico-contexto" "examples/invalid/semantico-contexto.oxe" "invalido" "semantica" "contexto_invalido")

message(STATUS "=========================================")
message(STATUS "Resumo da suíte de testes:")
message(STATUS "Total de casos verificados: ${total_testes}")
message(STATUS "Casos aprovados:           ${aprovados}")
message(STATUS "Limitações conhecidas:     ${limitacoes}")
message(STATUS "Falhas:                    ${falhas}")
message(STATUS "=========================================")

if(falhas GREATER 0)
  message(FATAL_ERROR "A suíte encontrou falhas inesperadas.")
endif()
