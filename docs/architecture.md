# Arquitetura futura

**Estado implementado:** somente documentação, exemplos de referência e pastas
reservadas. Não há lexer, parser, AST em código, verificador semântico, gerador,
runtime, build ou executável. Todas as etapas abaixo são planejamento.

## Fluxo previsto

```text
Código .oxe
    → Flex: tokens e localizações
    → Bison: reconhecimento sintático e construção explícita da AST
    → Análise semântica: AST e tabela de símbolos
    → Gerador Python 3 a partir da AST validada
    → Execução com Python 3
```

O destino vigente é **Python 3**; a implementação do tradutor está planejada
em **C++17**. A mudança de alvo foi decidida pelo grupo na OX-01, seguindo a
recomendação do professor para simplificar a geração e dispensar a compilação
da linguagem-alvo. Menções históricas a C++ nas fontes permanecem preservadas.
Flex e Bison serão usados conforme o [enunciado](references/projeto-disciplina.md).
O código C/C++ gerado pelo **Bison é a implementação do parser, não a AST**.
O parser deverá construir explicitamente nós que representem as construções da
Oxente Lang. Essa AST será reutilizada pela análise semântica e pela geração;
construí-la e descartá-la não atende ao projeto.

Visitor é uma abordagem arquitetural prevista para percorrer a AST em análises
e geração. Não existem classes Visitor prontas nem uma hierarquia definitiva.
Uma IR de baixo nível separada, otimizações e backend de máquina não fazem parte
do escopo obrigatório. A compilação em C++17 produzirá o tradutor; Python 3
executará os programas que ele gerar.

## Responsabilidades das pastas

| Diretório | Responsabilidade futura |
| --- | --- |
| `src/frontend/` | Especificações Flex/Bison, integração, tokens e leitura com localizações |
| `src/ast/` | Nós explícitos da AST e interfaces de percurso |
| `src/semantic/` | Tabela de símbolos, escopos, resolução de nomes, tipos e contextos |
| `src/codegen/` | Percurso da AST validada e emissão de Python 3 |
| `src/runtime/` | Apoio ao código gerado para preservar semântica de números, I/O e erros |
| `src/common/` | Localização de origem e diagnósticos compartilhados, conforme necessidade real |
| `examples/` | Programas documentados válidos e inválidos para validação futura |
| `tests/` | Testes futuros das etapas implementadas e da tradução |
| `scripts/` | Automações futuras que executem tarefas reais |

Os diretórios de implementação, testes e scripts contêm apenas `.gitkeep`.
Não há `CMakeLists.txt` nesta etapa: o build será configurado com fontes reais.
Artefatos de build e fontes geradas terão destinos ignorados conforme `.gitignore`.

O runtime poderá tratar vírgula e três casas decimais na saída de `tronxo`,
divisão real mesmo com operandos inteiros, verificações de zero, domínio e faixas
numéricas. O destino Python 3 não deve alterar regras da linguagem por acidente.
Precisão, limites e contratos pendentes serão confirmados antes da implementação.
Localizações `.oxe` devem acompanhar os nós relevantes para diagnósticos tanto
na análise quanto no programa gerado.

## Riscos da tradução para Python 3

Estes alertas orientam os cards de semântica, gerador e runtime. Não definem
helpers, nomes internos ou transformações de AST nesta tarefa.

| Diferença conhecida ou risco | Decisão ou contrato ainda pendente |
| --- | --- |
| Oxente tem escopo por bloco; blocos `if`/`while` em Python não criam escopo de variáveis | Preservar visibilidade e reutilização de nomes em blocos independentes; definir a estratégia nos cards futuros |
| `//` em Python arredonda para baixo; `%` acompanha essa divisão. Com negativos, isso difere do truncamento para zero citado na OX-01 | A cópia entregue (§2.3) não explicita negativos. Confirmar truncamento e regra do resto antes de gerar esses operadores diretamente |
| Um `continue` em `while` Python pula o restante do corpo; uma tradução de `arrodeia` pode assim pular sua atualização | Preservar a atualização ao tratar `simbora`; confirmar o contrato de `arrodeia` e a interação em laços aninhados antes de escolher a tradução |
| A saída padrão de Python não expressa automaticamente `tronxo` com vírgula e três casas nem `papo` como `evera`/`pala` | Confirmar arredondamento, representação booleana de saída, separadores e término de linha; especificar formatos de entrada e erros de execução com localização `.oxe` |
| Inteiros Python têm precisão arbitrária; reais e operações do destino podem não respeitar as faixas pretendidas na origem | Definir limites, precisão, domínio e diagnósticos numéricos; preservar divisão real, promoção e resultado `tronxo` de potência sem herdar tipos do destino |
| `and`/`or` em Python têm curto-circuito e retornam operandos; código auxiliar pode duplicar a avaliação de uma expressão | Confirmar curto-circuito e ordem de avaliação na origem; preservar tipos verificados e avaliar cada expressão uma única vez, sem duplicar efeitos ou leituras |

A [semântica](semantics.md#pontos-a-confirmar) mantém essas lacunas abertas.
A escolha do destino não aprova implicitamente comportamentos de Python como
regras da Oxente Lang.

## Roteiro incremental

1. Confirmar as lacunas registradas em [tokens](tokens.md), [gramática](grammar.md)
   e [semântica](semantics.md), mantendo a fonte entregue preservada.
2. Implementar o lexer Flex e testar tokens, comentários, textos e finais de linha.
3. Implementar o parser Bison e construção da AST explícita, verificar conflitos
   e a precedência com casos pequenos; configurar build real com as fontes.
4. Implementar tabela de símbolos e análise semântica por percursos da AST,
   incluindo escopos, tipos e contexto de laços.
5. Gerar Python 3 para o núcleo validado, acrescentar apoio de runtime necessário e
   verificar correspondência entre resultados de origem e execução do destino.
6. Documentar build do tradutor e execução real com Python 3, ampliar testes
   de casos válidos e inválidos; avaliar funções/procedimentos somente após o
   núcleo funcional.

O roteiro não representa trabalho funcional concluído na Sprint 0. Os
[marcos da disciplina](../README.md#cronograma-de-2026) orientam as próximas etapas.
