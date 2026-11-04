# Oxente Lang

Linguagem educacional do projeto da disciplina de Compiladores. A modalidade
escolhida é um **tradutor Oxente Lang → Python 3**, com implementação planejada em
**C++17**, usando **Flex** para análise léxica e **Bison** para análise sintática.

**Estado atual (OX-02):** especificação aprovada, build CMake e CLI disponíveis.
A integração mínima Flex/Bison comprova a compilação dos geradores; não implementa
a linguagem. Análise, AST, semântica e tradução ainda não estão disponíveis.

O pipeline planejado é: arquivo `.oxe` → Flex → Bison → AST explícita → análise
semântica/tabela de símbolos → gerador Python 3 → execução com Python 3.

A adoção de Python 3 como destino é uma decisão posterior do grupo, registrada
na OX-01, seguindo a recomendação do professor: simplifica a geração e dispensa
uma etapa de compilação da linguagem-alvo. O tradutor continua planejado em
C++17 com Flex/Bison; os programas traduzidos serão código Python 3.

## Preparação e build no Linux

Pré-requisitos: compilador C++17 (GCC 8+ ou Clang com biblioteca C++17),
CMake 3.16+, Make ou Ninja, Flex 2.6+ e Bison 3.0+. Python 3 será o destino
do tradutor futuro; não é necessário para compilar ou testar esta infraestrutura.

No Fedora:

```bash
sudo dnf install gcc-c++ cmake make flex bison m4 python3
```

No Debian/Ubuntu:

```bash
sudo apt-get update
sudo apt-get install g++ cmake make flex bison m4 python3
```

Na raiz do checkout (também no Codespaces):

```bash
cmake -S . -B build
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
./build/oxente --help
```

CMake exige as ferramentas e informa dependências ausentes. A configuração dentro
da árvore fonte é rejeitada: use sempre `-B build`. Fontes e headers gerados
ficam em `build/generated/`; não devem ser versionados. Para refazer do zero,
use uma nova pasta de build. CTest verifica argumentos, canais, códigos de saída,
preservação de arquivos e configuração/compilação em uma árvore limpa separada.
Os exemplos `.oxe` continuam referências manuais, sem validação pela linguagem.

## Preparação no Windows (WSL 2 + Ubuntu)

No Windows 10 atualizado ou Windows 11, use o WSL 2 com Ubuntu para executar
as mesmas ferramentas Linux do projeto. Esta é a rota documentada para Windows;
os comandos abaixo não exigem uma instalação nativa de Flex/Bison no PowerShell.

1. Abra o PowerShell **como administrador** e instale o WSL:

   ```powershell
   wsl --install -d Ubuntu
   ```

2. Reinicie o computador se solicitado. Abra o aplicativo **Ubuntu** e conclua
   a criação do usuário e senha Linux. Se o WSL já estiver instalado, confira
   a versão no PowerShell com `wsl --list --verbose`; se Ubuntu estiver na
   versão 1, execute `wsl --set-version Ubuntu 2`.

3. No terminal **Ubuntu/WSL**, instale as dependências e clone o projeto na pasta
   pessoal Linux (prefira `~/projetos` a `/mnt/c`):

   ```bash
   sudo apt-get update
   sudo apt-get install git g++ cmake make flex bison m4 python3
   mkdir -p ~/projetos
   cd ~/projetos
   git clone https://github.com/YannLeao/translator-project.git
   cd translator-project
   git switch feat/ox-02-build-cli
   cmake -S . -B build
   cmake --build build --parallel 2
   ctest --test-dir build --output-on-failure
   ./build/oxente --help
   ```

   Se já tiver um clone no WSL, entre nele e use `git fetch origin` antes de
   selecionar a branch, preservando suas alterações locais. Para repositório
   privado, autentique o Git conforme o acesso da sua conta GitHub.

Os comandos de build e execução devem continuar no Ubuntu/WSL, não no
PowerShell. Com VS Code, instale a extensão WSL e abra o checkout nesse ambiente.
Consulte a [instalação oficial do WSL](https://learn.microsoft.com/windows/wsl/install)
se a virtualização ou instalação do Ubuntu exigir configuração adicional.
A OX-02 foi testada em Linux; a instalação do WSL no Windows não foi executada
neste ambiente de desenvolvimento.

## Contrato da CLI (OX-02)

```text
oxente --help
oxente MODO arquivo.oxe [-o caminho]
```

Exija exatamente um modo explícito e uma entrada regular, legível, com extensão
`.oxe` (sensível a maiúsculas). Não há modo padrão. A ordem dos argumentos é
livre; `--` termina as opções, permitindo entradas iniciadas por hífen.
`--help` deve ser usado sozinho. Opções desconhecidas, modos repetidos ou
incompatíveis, múltiplas entradas e argumentos ausentes são erros.

| Opção | Contrato e estado |
| --- | --- |
| `--check` | Validação futura; não implementado |
| `--dump-ast` | Visualização futura da AST; não implementado |
| `--emit-python` | Emissão futura de Python 3; não implementado |
| `-o caminho`, `--output caminho` | Opcional, uma única vez e somente com `--emit-python`; não abre nem escreve saída nesta etapa |

Caminhos de saída iniciados por hífen devem usar `./nome`. A sintaxe
`--output=caminho` não é suportada. O destino futuro sem `-o` permanece pendente;
esta versão sempre falha explicitamente nos três modos, sem gerar arquivos nem
invocar o parser provisório. A existência/extensão da entrada é conferida antes
de informar a capacidade pendente; isso não valida seu conteúdo.

| Código | Significado |
| --- | --- |
| 0 | Ajuda exibida em stdout |
| 2 | Erro de argumentos/uso, incluindo extensão inválida |
| 3 | Entrada inexistente, não regular ou impossível de abrir |
| 4 | Funcionalidade não implementada |

Erros vão para stderr, sem saída normal em stdout. São códigos iniciais da CLI,
sem antecipar a padronização de diagnósticos da OX-10. Por exemplo:

```bash
./build/oxente --check examples/valid/calculo.oxe
# stderr: oxente: --check: não implementado; nenhum programa foi analisado ou traduzido.
# código de saída: 4
```

A integração usa o esqueleto C do Bison compilado como C++17 e o scanner C do
Flex também compilado como C++, com prefixo privado `oxinfra`. Não depende de
`libfl`: `noyywrap` remove esse requisito. O scanner provisório devolve um único
marcador de indisponibilidade e a produção sempre aborta, inclusive rejeitando
entrada vazia. Os símbolos são ligados ao executável, mas a CLI não os chama.
OX-04/OX-05 deverão substituir esses arquivos e definir tokens, localizações,
contexto e interfaces reais; o prefixo provisório não constitui API estável.
OX-03 permanece responsável pela AST, que não é o código gerado pelo Bison.

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

Integrantes:

- Adriano Júnior
- Maria Clara Ferreira
- Yann Leão

A disciplina permite apoio de IA. Os responsáveis devem revisar e compreender
o material, explicar as decisões e declarar as ferramentas e atividades em que
foram utilizadas. Registro de ferramentas e atividades: _a preencher pelo grupo_.

Alterações ficam locais para revisão. Editar arquivos não autoriza commits,
pushes, merges, abertura de PR/MR ou publicação.
