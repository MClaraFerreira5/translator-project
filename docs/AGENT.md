# Guia para agentes e colaboradores

## Escopo e fonte de verdade

Oxente Lang é um projeto acadêmico de tradutor para Python 3, com implementação
planejada em C++17, Flex e Bison. A mudança de destino foi decidida pelo grupo
na OX-01, após a proposta entregue; as fontes históricas permanecem intactas.
Consulte a [definição entregue](references/definicao-entregue.md),
fonte principal, e o [enunciado da disciplina](references/projeto-disciplina.md).
A [procedência](references/README.md) das cópias deve continuar documentada.
Não substitua a versão entregue por rascunhos nem corrija a fonte silenciosamente.

A Sprint 0 contém apenas documentação, estrutura e exemplos. Não criar lexer,
parser, classes C++, build, CLI, CI ou testes executáveis nesta etapa. Tarefas
futuras poderão implementar etapas reais de forma incremental, com validação
proporcional e documentação do que efetivamente funciona.

## Decisões e consistência

Use `inteiro`, `tronxo`, `papo`, `resenha` e `arrodeia`. Na extensão de funções,
use `aperreio`, `toma`, `sem_futuro`, conforme a tabela mais recente. Não reative
`int`, `float`, `bool`, `string`, `Arrodeia`, `funcao`, `devolve` ou `void` como
palavras-chave da origem. Registre a divergência quando a fonte usar grafias antigas.

Não invente decisões: separe regra aprovada, esboço e ponto a confirmar. `liso` e
`balaio` não têm especificação completa. Funções/procedimentos são extensões após
o núcleo; não ampliam automaticamente o escopo obrigatório. Consulte os pontos
a confirmar em [tokens](tokens.md), [gramática](grammar.md) e [semântica](semantics.md).

Todo material novo deve estar em português brasileiro. Exemplos `.oxe` devem
usar ASCII, palavras canônicas e instruções em linhas reais. Casos inválidos
devem isolar uma falha e ter diagnóstico esperado no índice. Não afirme execução
ou validação automática que não ocorreu. Preserve os artefatos da fonte original.

## Arquitetura e evolução

Siga a [arquitetura planejada](architecture.md): tokens → AST explícita → análise
semântica/tabela de símbolos → gerador Python 3 → execução com Python 3.
O código gerado pelo Bison não é a AST.
Visitor está previsto para percursos; IR separada e otimizações não são obrigatórias.
Mantenha responsabilidades de `src/*` delimitadas e considere o runtime para
preservar regras próprias, evitando herdar acidentalmente a semântica de Python.

Ao alterar uma regra confirmada, atualize especificação, tokens, gramática,
semântica e exemplos afetados. Verifique links e diferencie comportamento
planejado de comportamento implementado. Não crie árvores duplicadas ou
dependências sem necessidade. Use `.gitkeep` somente para diretórios úteis vazios.

## Revisão e aprovação

Inspecione estado Git, instruções e mudanças locais antes de editar. Preserve
trabalho do usuário; não use operações destrutivas para limpar o repositório.
Edição autorizada permite preparar alterações revisáveis. Decisões novas ou
ambíguas devem ser registradas e submetidas ao grupo/professor, sem apresentá-las
como parte aprovada da linguagem.

Não executar `git add` automaticamente. **Não fazer commits, pushes ou merges
sem aprovação explícita do usuário, mesmo quando uma tarefa estiver concluída.**
**Consentimento para editar arquivos não é consentimento para commitar.**
Não abrir PR/MR nem publicar sem autorização explícita. Antes de encerrar,
inspecione `git status`, `git diff` e arquivos não rastreados; relate validações,
limitações e pendências. Deixe as alterações locais aguardando revisão.
