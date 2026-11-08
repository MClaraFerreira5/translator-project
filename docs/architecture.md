# Arquitetura

**Implementado:** build CMake, CLI com `--dump-ast`, scanner Flex, parser do núcleo,
AST, Visitor, diagnósticos, tabela de símbolos com análise de escopos, gerador
Python 3 e runtime.
**Planejado:** análise de tipos e de contexto de laços, e a ligação de `--check` e
`--emit-python`. Por isso a CLI ainda não traduz arquivos `.oxe`. Veja o
[estado por etapa](../README.md#estado-atual) e o
[contrato da CLI e build](../README.md#contrato-da-cli-ox-02).

## Pipeline

```text
Código .oxe
    → Flex: tokens e localizações
    → Bison: reconhecimento sintático e construção explícita da AST
    → Análise semântica: AST e tabela de símbolos
    → Gerador Python 3 a partir da AST validada
    → Execução com Python 3
```

O destino vigente é **Python 3**; o tradutor é implementado em **C++17**. A mudança de alvo foi decidida pelo grupo na OX-01, seguindo a
recomendação do professor para simplificar a geração e dispensar a compilação
da linguagem-alvo. Menções históricas a C++ nas fontes permanecem preservadas.
Flex e Bison serão usados conforme o [enunciado](references/projeto-disciplina.md).
O código C/C++ gerado pelo **Bison é a implementação do parser, não a AST**.
O parser deverá construir explicitamente nós que representem as construções da
Oxente Lang. Essa AST será reutilizada pela análise semântica e pela geração;
construí-la e descartá-la não atende ao projeto.

Uma IR de baixo nível separada, otimizações e backend de máquina não fazem parte
do escopo obrigatório. A compilação em C++17 produz o tradutor; Python 3 executa
os programas gerados.

## AST, Visitor e tabela de símbolos

A AST fica em `src/ast/`. `ProgramNode` é a raiz e guarda o bloco principal;
`StmtNodes.hpp` define os comandos e `ExprNodes.hpp`, as expressões. Cada nó é dono
dos filhos por `std::unique_ptr` e guarda a localização de origem. `desembucha`
tem uma lista de argumentos, `abs` é um `UnaryOp` e `//` é `BinaryOp::DivisaoInteira`.
`se_num_for` é representado por um `IfStmtNode` como único comando do `senao_bloco`
do ramo anterior.

`AstVisitor` declara um `visit` por tipo de nó, e cada nó despacha em `accept`.
Há três visitantes de produção: `ScopeVisitor`, `PythonGenerator` e `AstPrinter`, que
implementa `--dump-ast`. O teste `pipeline` passa a mesma AST pelos três.

`SymbolTable` mantém a pilha de escopos, com nome, tipo, localização e nível de cada
símbolo, e rejeita sombreamento. `ScopeVisitor` abre um escopo por bloco e outro
envolvendo o `arrodeia`. O campo `tipo_calculado` das expressões só é preenchido em
literais, pois a análise de tipos é planejada. Por isso o gerador deduz os tipos
de que precisa a partir da declaração das variáveis e da tabela de operadores.

## Responsabilidades das pastas

| Diretório | Responsabilidade |
| --- | --- |
| `src/frontend/` | Especificações Flex/Bison, integração, tokens e leitura com localizações |
| `src/ast/` | Nós explícitos da AST e interfaces de percurso |
| `src/semantic/` | Tabela de símbolos, escopos, resolução de nomes, tipos e contextos |
| `src/codegen/` | Percurso da AST validada e emissão de Python 3 |
| `src/runtime/` | Apoio ao código gerado para preservar semântica de números, I/O e erros |
| `src/common/` | Localização de origem e diagnósticos compartilhados, conforme necessidade real |
| `examples/` | Programas documentados válidos e inválidos |
| `tests/` | Testes das etapas implementadas e da tradução |
| `scripts/` | Automação de build e testes |

A OX-02 acrescenta `src/main.cpp`, integração mínima em `src/frontend/`,
`CMakeLists.txt` e testes de infraestrutura/CLI em `tests/`.
Artefatos ficam na árvore de build ignorada pelo Git.
A OX-04 implementa o scanner em `infra.l`, tokens e contrato mínimo em `infra.y`,
encapsulamento em `scanner.hpp`/`scanner.cpp` e inspeção em `tokens_main.cpp`.
As OX-05 e OX-06 acrescentam a `infra.y` as expressões, os comandos, os blocos, os
condicionais e os laços, com `oxente::parse` como ponto de entrada. O [contrato do scanner](../README.md#scanner-e-inspeção-de-tokens-ox-04)
documenta propriedade de valores, localizações e limitações para OX-05/OX-07.

O runtime trata vírgula e três casas decimais na saída de `tronxo`, `papo` como
`evera`/`pala`, divisão real mesmo com operandos inteiros, zero nas divisões,
domínio da potência e entrada por tipo. Faixas numéricas ainda não são verificadas.
O destino Python 3 não deve alterar regras da linguagem por acidente.
Os helpers de divisão e potência recebem a linha `.oxe` para os diagnósticos.

## Gerador Python 3 (OX-11 e OX-12)

`src/codegen/PythonGenerator` traduz uma AST validada para Python 3. É testado com
ASTs manuais em `tests/test_codegen.cpp` e a partir de código `.oxe` em
`tests/teste_pipeline.cpp`; ambos executam os programas gerados.
Ainda não está ligado à CLI: `--emit-python` depende da integração do pipeline (OX-15). O programa gerado importa
`oxente_runtime`, que precisa estar ao lado dele ou no `PYTHONPATH`. Uma AST que
viole o contrato, como nome não declarado, gera `ErroGeracao`: é erro interno do
tradutor, e não diagnóstico para o usuário.

| Construção | Tradução |
| --- | --- |
| Variável | `v<N>_<nome>`, com `N` contando as declarações do nome; não colide com palavras reservadas, builtins ou helpers `_ox_*` e isola blocos independentes |
| Promoção para `tronxo` | `float(...)` na declaração e na atribuição |
| `/`, `//`, `%`, `**` | Helpers do runtime, com a linha de origem |
| `se_num_for` | `IfStmtNode` como único comando de `senao_bloco`; emitido como `elif` |
| `num_para ate (c)` | `while not (c):` |
| `arrodeia` | Inicialização, `while not (c):`, corpo e atualização no fim do corpo |
| `simbora` em `arrodeia` | Atualização do laço mais interno, seguida de `continue` |

**Escolhas provisórias, a confirmar com o grupo:** um espaço entre argumentos de
`desembucha` (única referência é `Resultado 14`, no exemplo 4.1 da proposta);
curto-circuito de `e`/`ou`, herdado de Python; `canso` sem atualização; escopo
próprio envolvendo o `arrodeia`, como em `ScopeVisitor`; truncamento para zero
em `//` e `%`, já adotado pelo runtime.

## Riscos da tradução para Python 3

| Diferença conhecida ou risco | Tratamento atual e o que falta confirmar |
| --- | --- |
| Oxente tem escopo por bloco; blocos `if`/`while` em Python não criam escopo de variáveis | Resolvido com a renomeação `v<N>_<nome>` por declaração |
| `//` em Python arredonda para baixo; `%` acompanha essa divisão. Com negativos, isso difere do truncamento para zero citado na OX-01 | Os helpers truncam para zero. A cópia entregue (§2.3) não explicita negativos: falta confirmar a regra |
| Um `continue` em `while` Python pula o restante do corpo; uma tradução de `arrodeia` pode assim pular sua atualização | A atualização é emitida antes de cada `continue`. Falta confirmar o contrato de `arrodeia` |
| A saída padrão de Python não expressa automaticamente `tronxo` com vírgula e três casas nem `papo` como `evera`/`pala` | `_ox_formatar_valor` aplica o formato. Falta confirmar arredondamento, separadores, término de linha e formatos de entrada |
| Inteiros Python têm precisão arbitrária; reais e operações do destino podem não respeitar as faixas pretendidas na origem | Divisão real, promoção e `tronxo` de potência são preservados. Limites e precisão não estão definidos nem verificados |
| `and`/`or` em Python têm curto-circuito e retornam operandos; código auxiliar pode duplicar a avaliação de uma expressão | Operandos validados são `papo`, então o resultado também é. Cada expressão roda uma vez por caminho. Falta confirmar o curto-circuito |

A [semântica](semantics.md#pontos-a-confirmar) mantém essas lacunas abertas.
A escolha do destino não aprova implicitamente comportamentos de Python como
regras da Oxente Lang.

## Roteiro incremental

| Etapa | Estado |
| --- | --- |
| Lexer Flex e testes | Concluído |
| Tabela de símbolos e escopos | Concluído |
| Gerador Python 3 e runtime, com ASTs manuais | Concluído |
| Parser Bison do núcleo e `--dump-ast` | Concluído |
| Análise de tipos e de contexto de laços | Planejado |
| Ligação de `--check` e `--emit-python` na CLI | Planejado |
| Confirmar as lacunas de [tokens](tokens.md), [gramática](grammar.md) e [semântica](semantics.md) | Em aberto |
| Funções e procedimentos | Somente após o núcleo funcional |

Os [marcos da disciplina](../README.md#cronograma-de-2026) orientam as próximas etapas.
