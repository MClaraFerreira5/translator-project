# Registro de revisão — documentação e demonstração

Preparação local em **09/11/2026**, na branch
`docs/ox-16-ox-17-apresentacao`, a partir de `origin/main` em
`75a6461272f2f3827ed4c653998ead0d6dd6d171` (PR #29 integrada).
`git fetch origin main` confirmou que a base local correspondia à remota.
O destino é `https://github.com/MClaraFerreira5/translator-project.git`.
A pasta local `temp/` preexistente foi preservada; as fontes históricas em
`docs/references/` não foram alteradas.

## Bloco de revisão #17 / OX-16 — documentação

Arquivos deste conjunto:

- `README.md`: capa com proposta, exemplo real, início rápido e navegação.
- `docs/README.md`, `docs/cli.md`, `docs/preparacao.md`, `docs/frontend.md`:
  índice e guias com os detalhes técnicos que antes interrompiam a capa.
- `docs/architecture.md`, `docs/tokens.md`, `docs/grammar.md`,
  `docs/semantics.md`, `docs/language-spec.md`, `docs/AGENT.md`:
  links e descrições alinhados ao núcleo já implementado, preservando pendências.
- `docs/test-suite.md`, `examples/README.md`: alcance real dos testes,
  comparações de saída e limites dos runners existentes.
- Remoção de `src/ast/.gitkeep`, `src/common/.gitkeep`,
  `src/runtime/.gitkeep` e `src/semantic/.gitkeep`: os quatro diretórios já
  possuem arquivos versionados. Não havia marcador útil em diretório vazio.

O URL de clone no guia WSL foi corrigido para o remoto conferido. Créditos e
pendência da declaração de IA foram mantidos. O enunciado continua guardando as
datas; o README não repete cronograma nem balanços por card.

## Bloco de revisão #18 / OX-17 — demonstração

Arquivos deste conjunto:

- `src/frontend/infra.l`: padrões/ações alinhados, ações extensas divididas e
  comentários de estados, términos, operadores, erros e transporte de tokens.
- `src/frontend/infra.y`, `src/frontend/tokens_main.cpp`, `src/main.cpp`:
  comentários e espaçamento pontuais para a leitura do pipeline.
- `docs/demonstracao.md`: comandos copiáveis, mapa pesquisável do código,
  exercício de 1 a 10, três erros isolados, divisão sugerida e ensaio.
- `docs/revisao-apresentacao.md`: este registro de evidências dos dois blocos.

A mudança no código é de apresentação: padrões, ordem de regras, estados,
tokens, produções, propriedade de valores, localizações e saídas foram preservados.
Não foi acrescentado exemplo versionado: o exercício é escrito no build, então
índice e manifesto continuam descrevendo os mesmos 34 exemplos.

A entrega está dividida em dois commits na mesma branch, um por conjunto,
conforme autorização do usuário para commits e push. A mensagem sugerida da PR
fica em `temp/mensagem-pr-ox-16-ox-17.md`. A revisão final, a escolha dos revisores
e o merge cabem ao usuário; a abertura da PR não faz parte desta preparação.

## Ambiente e evidências

Ambiente: Fedora Linux 45, GCC 16.2.1, CMake 4.3.0, Flex 2.6.4, Bison 3.8.2.
O CMake selecionou Python 3.12.13; os comandos `python3` do roteiro executaram
Python 3.15.0. Não foi necessário instalar dependências neste ambiente.

| Verificação | Resultado local |
| --- | --- |
| Base antes de editar código: build novo em `build/base-apresentacao`, `BUILD_TESTING=ON` | 14/14 testes CTest |
| Versão organizada: build novo em `build/apresentacao`, `BUILD_TESTING=ON` | 14/14 testes CTest |
| Início rápido literal do README, desde a configuração nova em `build/` | 14/14 testes; validação e Python com `Resultado 14` e `Com parenteses 20` |
| Catálogo, saída detalhada de `suite` | 34 aprovados, zero pendências e zero falhas |
| Ponta a ponta pela CLI (`e2e`) | 67/67 casos sem falha |
| Comparação de tokens base × versão organizada | 13/13 casos idênticos em stdout, stderr e código |
| Comandos do roteiro: tokens, AST, análise, emissão com `-o` e Python | Executados na raiz; saídas conferidas |
| Exercício escrito no build | Dez linhas, exatamente de `1` a `10`, sem stderr, código 0 |
| Erros léxico, sintático e semântico | Código 1, stdout vazio e etapa/causa/localização em stderr |
| Bison com conflitos tratados como erro | Zero conflitos de deslocamento/redução e redução/redução |
| Links relativos e âncoras Markdown | Conferidos em README, AGENTS, docs e índice de exemplos |
| `git diff --check` | Sem erros |

Comandos usados para as suítes (na base e depois na versão organizada):

```bash
cmake -S . -B build/base-apresentacao -DBUILD_TESTING=ON
cmake --build build/base-apresentacao --parallel 2
ctest --test-dir build/base-apresentacao --output-on-failure
cmake -S . -B build/apresentacao -DBUILD_TESTING=ON
cmake --build build/apresentacao --parallel 2
ctest --test-dir build/apresentacao --output-on-failure
ctest --test-dir build/apresentacao -R '^(suite|e2e)$' --verbose
bison -Werror=conflicts-sr -Werror=conflicts-rr --report=state --report-file=build/apresentacao/verificacao-bison.txt --defines=build/apresentacao/verificacao-bison.h -o build/apresentacao/verificacao-bison.cpp src/frontend/infra.y
git diff --check
```

Os 13 cenários de comparação de tokens exercitam reservadas e identificadores,
operadores longos, números e texto, comentários com LF e CRLF, EOF sem quebra,
caractere inválido, texto/comentário não terminado e os casos pendentes de
comentário de linha no EOF, CR isolado, adjacência numérica e texto vazio.
As duas versões leram os mesmos caminhos para comparar também os diagnósticos
sem normalizar stderr. Capturas por caso e resumo estão em
`build/comparacao-tokens/`; os comandos do roteiro foram extraídos dos blocos
Markdown e seus registros ficaram em `build/apresentacao/roteiro-*.log`.
O início rápido ficou em `build/inicio-rapido.log`. Esses artefatos são locais,
ignorados pelo Git e não fazem parte da entrega versionada.

## Achados separados e pendências

Nenhum defeito funcional novo foi corrigido nesta tarefa. Permanecem observações
sobre infraestrutura existente: `suite.cmake` lista os casos explicitamente e
confere sobretudo códigos de saída; os runners do catálogo ainda aceitam como
pendente um modo indisponível (código 4) ou binário ausente. O CTest desta revisão
não teve nenhuma pendência; `e2e` complementa as verificações de saídas e
mensagens. Essas limitações estão descritas no guia de testes, sem alterar o
comportamento dos runners nesta tarefa de organização.

As decisões provisórias da linguagem seguem em tokens, gramática e semântica.
Faixas inteiras, detalhes de textos, I/O e `arrodeia` não foram decididos aqui.
A mensagem sintática permanece genérica; o roteiro explica a causa com o exemplo.

Dependem do grupo: ensaio coletivo, divisão nominal da fala, teste na máquina de
apresentação/WSL, conferência da projeção e registro consolidado de IA. A lista
operacional está no [roteiro](demonstracao.md#lista-de-checagem).
