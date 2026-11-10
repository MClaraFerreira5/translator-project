# Suíte de testes

Execute na raiz do repositório com as [dependências](preparacao.md) instaladas:

```bash
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

`sh scripts/build-e-testes.sh` executa configuração, build e CTest. A CI usa esse
script. Use uma pasta nova para validar a compilação desde o início.

## Organização e alcance

| Teste CTest | Verificação |
| --- | --- |
| `lexer` | Tokens, lexemas, localizações, reservadas, operadores, estados, LF/CRLF/EOF e falhas |
| `lexer_harness` | Códigos e canais de `oxente-tokens` |
| `cli` | Argumentos, arquivos, modos, códigos de saída e preservação de saída em erros |
| `build_limpo` | Configuração e compilação em árvore separada, sem testes |
| `diagnosticos` | Formatação, etapas, localização e motor de diagnósticos |
| `suite` | Aceitação/rejeição dos exemplos pelo scanner ou `--check` |
| `scope` | Tabela de símbolos, blocos, nomes visíveis e sombreamento |
| `parser` | AST real, precedência, associatividade, comandos, blocos, laços e entradas inválidas |
| `semantic` | Tipos, nomes e contexto de laços em ASTs manuais |
| `codegen` | ASTs manuais → Python → execução e comparação da saída |
| `runtime` | Entrada/saída, divisões, potência, conversões e resultados reais não finitos |
| `ast_manual` | Construção explícita dos nós da AST |
| `pipeline` | Parser → AST → análise de escopos → geração e execução Python |
| `e2e` | CLI real → Python autossuficiente → stdout, stderr e código de saída |

O [catálogo](../tests/suite_manifest.json) descreve os exemplos válidos e inválidos,
etapas responsáveis, tópicos e diagnósticos esperados. O [índice de exemplos](../examples/README.md)
explica cada programa. `suite.cmake` mantém chamadas explícitas dos casos; não
interpreta o manifesto JSON. `e2e.py` consulta o manifesto para os inválidos e mantém
as saídas completas dos válidos em `EXEMPLOS`.

`e2e` também cobre entradas dos quatro tipos, erros de execução, escopos, divisão
negativa, laços com `simbora`, curto-circuito e regressões numéricas. Cada Python
roda fora da árvore fonte, sem `PYTHONPATH`, para verificar o runtime embutido.
`codegen`, `pipeline` e `e2e` têm limites de tempo no CTest para detectar laços que
não terminam.

## Executar partes da suíte

```bash
ctest --test-dir build -R '^(lexer|lexer_harness|parser)$' --output-on-failure
ctest --test-dir build -R '^(suite|e2e)$' --verbose
python3 tests/e2e.py ./build/oxente build/e2e-manual
```

O runner alternativo `python3 tests/executar_suite.py` procura binários em `build/`
ou na raiz; não encontra automaticamente builds em subpastas. Ele verifica o
catálogo, mas não substitui o CTest nem a execução do Python gerado.

## Limitações e leitura dos resultados

A presença de um tópico no catálogo indica cobertura por casos, não prova de
cobertura completa da especificação. `suite` verifica principalmente códigos de
saída; `e2e` confere também os diagnósticos dos inválidos e a saída dos válidos.
Os runners do catálogo ainda contêm tratamento histórico para modo indisponível
(código 4) e binários ausentes. Na validação atual não deve haver casos pulados ou
pendentes; confira os totais na saída detalhada.

A faixa inteira da linguagem e as decisões provisórias de textos, comentários,
`arrodeia` e I/O continuam abertas na especificação. Os testes validam o
comportamento atual, sem aprovar essas decisões. WSL e máquina de apresentação
precisam de verificação pelo grupo. Há uma falha conhecida em `lexer_harness` no
build nativo MSYS2.

As evidências desta preparação estão no [registro de revisão](revisao-apresentacao.md).
