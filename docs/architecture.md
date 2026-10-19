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
    → Geração de C++ a partir da AST validada
    → Compilador C++
    → Executável
```

O destino escolhido é C++; a implementação do tradutor está planejada em C++17.
Flex e Bison serão usados conforme o [enunciado](references/projeto-disciplina.md).
O código C/C++ gerado pelo **Bison é a implementação do parser, não a AST**.
O parser deverá construir explicitamente nós que representem as construções da
Oxente Lang. Essa AST será reutilizada pela análise semântica e pela geração;
construí-la e descartá-la não atende ao projeto.

Visitor é uma abordagem arquitetural prevista para percorrer a AST em análises
e geração. Não existem classes Visitor prontas nem uma hierarquia definitiva.
Uma IR de baixo nível separada, otimizações e backend de máquina não fazem parte
do escopo obrigatório. O compilador C++ externo produzirá o executável futuro.

## Responsabilidades das pastas

| Diretório | Responsabilidade futura |
| --- | --- |
| `src/frontend/` | Especificações Flex/Bison, integração, tokens e leitura com localizações |
| `src/ast/` | Nós explícitos da AST e interfaces de percurso |
| `src/semantic/` | Tabela de símbolos, escopos, resolução de nomes, tipos e contextos |
| `src/codegen/` | Percurso da AST validada e emissão de C++ |
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
numéricas. O destino C++ não deve alterar regras da linguagem por acidente.
Precisão, limites e contratos pendentes serão confirmados antes da implementação.
Localizações `.oxe` devem acompanhar os nós relevantes para diagnósticos tanto
na análise quanto no programa gerado.

## Roteiro incremental

1. Confirmar as lacunas registradas em [tokens](tokens.md), [gramática](grammar.md)
   e [semântica](semantics.md), mantendo a fonte entregue preservada.
2. Implementar o lexer Flex e testar tokens, comentários, textos e finais de linha.
3. Implementar o parser Bison e construção da AST explícita, verificar conflitos
   e a precedência com casos pequenos; configurar build real com as fontes.
4. Implementar tabela de símbolos e análise semântica por percursos da AST,
   incluindo escopos, tipos e contexto de laços.
5. Gerar C++ para o núcleo validado, acrescentar apoio de runtime necessário e
   verificar correspondência entre resultados de origem e execução do destino.
6. Documentar compilação/execução reais e ampliar testes de casos válidos e
   inválidos; avaliar funções/procedimentos somente após o núcleo funcional.

O roteiro não representa trabalho funcional concluído na Sprint 0. Os
[marcos da disciplina](../README.md#cronograma-de-2026) orientam as próximas etapas.
