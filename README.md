# Oxente Lang

Linguagem educacional do projeto da disciplina de Compiladores. A modalidade
escolhida é um **tradutor Oxente Lang → Python 3**, com implementação planejada em
**C++17**, usando **Flex** para análise léxica e **Bison** para análise sintática.

**Estado atual:** especificação aprovada; estrutura e documentação em preparação;
implementação ainda não iniciada. Não há tradutor, build ou comandos de execução
disponíveis nesta etapa.

O pipeline planejado é: arquivo `.oxe` → Flex → Bison → AST explícita → análise
semântica/tabela de símbolos → gerador Python 3 → execução com Python 3.

A adoção de Python 3 como destino é uma decisão posterior do grupo, registrada
na OX-01, seguindo a recomendação do professor: simplifica a geração e dispensa
uma etapa de compilação da linguagem-alvo. O tradutor continua planejado em
C++17 com Flex/Bison; os programas traduzidos serão código Python 3.

## Escopo

O núcleo prioritário para **12/11/2026** contempla expressões aritméticas, variáveis,
atribuição, entrada/saída, condicionais, laços, análises léxica/sintática/semântica,
AST explícita, tabela de símbolos e tradução para Python 3. A proposta aprovada
detalha esse núcleo com quatro tipos, operadores lógicos, escopos e controles
de laço.

Funções, procedimentos, parâmetros e retornos são **extensões planejadas após o
núcleo funcional**. `liso` e `balaio` são itens pendentes, sem regras completas.
IR separada da AST, backend de máquina e otimizações não são requisitos do projeto.

## Organização e leitura

```text
.
├── README.md
├── AGENTS.md
├── docs/
│   ├── AGENT.md
│   ├── language-spec.md
│   ├── tokens.md
│   ├── grammar.md
│   ├── semantics.md
│   ├── architecture.md
│   └── references/
├── examples/
│   ├── README.md
│   ├── valid/
│   └── invalid/
├── src/
│   ├── frontend/
│   ├── ast/
│   ├── semantic/
│   ├── codegen/
│   ├── runtime/
│   └── common/
├── tests/
└── scripts/
```

Leia a [especificação consolidada](docs/language-spec.md), o
[catálogo de tokens](docs/tokens.md), o [esboço de gramática](docs/grammar.md),
as [regras semânticas](docs/semantics.md) e a [arquitetura futura](docs/architecture.md).
Os [exemplos e resultados esperados](examples/README.md) são referências manuais
para implementação futura; ainda não foram executados por um tradutor.

As [fontes e sua procedência](docs/references/README.md) estão preservadas para
consulta. Colaboradores e agentes devem ler [AGENTS.md](AGENTS.md) e
[docs/AGENT.md](docs/AGENT.md) antes de editar.

## Cronograma de 2026

| Data | Entrega |
| --- | --- |
| 22/10/2026 | Apresentação da proposta e especificação inicial |
| 12/11/2026 | Versão intermediária funcional |
| 26/11/2026 | Entrega final e apresentação |

Datas conforme o [enunciado da disciplina](docs/references/projeto-disciplina.md).

## Equipe e uso de IA

Integrantes: _a preencher pelo grupo_.

A disciplina permite apoio de IA. Os responsáveis devem revisar e compreender
o material, explicar as decisões e declarar as ferramentas e atividades em que
foram utilizadas. Registro de ferramentas e atividades: _a preencher pelo grupo_.

Alterações ficam locais para revisão. Editar arquivos não autoriza commits,
pushes, merges, abertura de PR/MR ou publicação.
