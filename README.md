# Oxente Lang

Linguagem educacional do projeto da disciplina de Compiladores. A modalidade
escolhida é um **tradutor Oxente Lang → Python 3**, implementado em **C++17**,
usando **Flex** para análise léxica e **Bison** para análise sintática.

## Estado atual

Parser, AST, escopos e gerador já funcionam juntos a partir de código `.oxe`, como
mostra o teste `pipeline`. **A CLI ainda não traduz arquivos**: só `--dump-ast` está
ligado; `--check` e `--emit-python` dependem da OX-15.

| Etapa | Estado | Onde |
| --- | --- | --- |
| Build, CLI (argumentos e códigos de saída) | Implementado | `CMakeLists.txt`, `src/main.cpp` |
| Scanner Flex | Implementado | `src/frontend/infra.l`, `oxente-tokens` |
| AST e contrato Visitor | Implementado | `src/ast/` |
| Diagnósticos | Implementado | `src/common/` |
| Tabela de símbolos e escopos | Implementado, fora da CLI | `src/semantic/` |
| Gerador Python 3 | Implementado, testado com ASTs manuais | `src/codegen/` |
| Runtime Python | Implementado | `src/runtime/oxente_runtime.py` |
| Parser do núcleo, com blocos, condicionais e laços | Implementado | `src/frontend/infra.y` |
| Visualização da AST (`--dump-ast`) | Implementado | `src/ast/AstPrinter.cpp` |
| Análise de tipos e contexto de laços | Planejado | OX-09 |
| `--check`, `--emit-python` | Planejado; respondem "não implementado" | OX-15 |
| Funções e procedimentos | Não suportado; extensão após o núcleo | `aperreio`, `toma`, `sem_futuro` |
| `liso` e `balaio`, vetores, IR, otimizações | Não suportado | — |

## Arquitetura

```text
arquivo .oxe → Flex → Bison → AST explícita → análise semântica/tabela de
símbolos → gerador Python 3 → execução com Python 3 e oxente_runtime
```

Detalhes da AST, do Visitor, da tabela de símbolos e das decisões de tradução estão
em [docs/architecture.md](docs/architecture.md).

A adoção de Python 3 como destino é uma decisão posterior do grupo, registrada
na OX-01, seguindo a recomendação do professor: simplifica a geração e dispensa
uma etapa de compilação da linguagem-alvo.

## Preparação e build no Linux

Pré-requisitos: compilador C++17 com `std::to_chars` para `double` (GCC 11+),
CMake 3.16+, Make ou Ninja, Flex 2.6+, Bison 3.0+ e Python 3. Python é exigido
quando os testes estão ativos e para executar os programas gerados; para compilar
sem ele, configure com `-DBUILD_TESTING=OFF`.

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

Os três primeiros comandos estão reunidos em `sh scripts/build-e-testes.sh`, que
aceita a pasta de build como argumento. O mesmo script é usado pela CI em
`.github/workflows/ci.yml`.

CMake exige as ferramentas e informa dependências ausentes. A configuração dentro
da árvore fonte é rejeitada: use sempre `-B build`. Fontes e headers gerados
ficam em `build/generated/`; não devem ser versionados. Para refazer do zero,
use uma nova pasta de build. CTest verifica a CLI, o scanner, os diagnósticos, a
AST, os escopos, o gerador, o runtime e a configuração/compilação em uma árvore
limpa separada. Veja a [suíte de testes](docs/test-suite.md). Os exemplos `.oxe`
continuam referências manuais, sem validação de ponta a ponta.

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
   git switch main
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
neste ambiente de desenvolvimento. Em build nativo com MSYS2, fora da rota
documentada, o teste `lexer_harness` falha com "CR isolado"; os demais passam.

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
| `--dump-ast` | Exibe a AST como texto indentado em stdout, sem análise semântica |
| `--emit-python` | Emissão futura de Python 3; não implementado |
| `-o caminho`, `--output caminho` | Opcional, uma única vez e somente com `--emit-python`; não abre nem escreve saída nesta etapa |

Caminhos de saída iniciados por hífen devem usar `./nome`. A sintaxe
`--output=caminho` não é suportada. O destino futuro sem `-o` permanece pendente;
`--check` e `--emit-python` sempre falham explicitamente, sem gerar arquivos nem
invocar o parser. A existência/extensão da entrada é conferida antes
de informar a capacidade pendente; isso não valida seu conteúdo.

| Código | Significado |
| --- | --- |
| 0 | Ajuda ou AST exibida em stdout |
| 1 | Erro léxico ou sintático no programa |
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

`--dump-ast` mostra um nó por linha, e a indentação indica os filhos. Um `.oxe` com
`inteiro r bota 2 + 3 * 4` gera:

```text
Programa
  Bloco
    Declaracao inteiro r
      Binario +
        Literal inteiro 2
        Binario *
          Literal inteiro 3
          Literal inteiro 4
```

## Gerar e executar Python

Enquanto a CLI não está integrada, `--emit-python` não gera arquivos.
O gerador é usado como biblioteca, a partir de uma AST validada:

```cpp
oxente::PythonGenerator gerador;
std::string codigo_python = gerador.gerar(programa); // programa: ProgramNode&
```

O módulo gerado importa `oxente_runtime`, então esse arquivo precisa estar ao lado
do programa ou no `PYTHONPATH`:

```bash
PYTHONPATH=src/runtime python3 programa.py
```

Para ver programas gerados de verdade, rode o teste do gerador e abra os `.py`
produzidos. Eles têm duas linhas iniciais, próprias do teste, que localizam o runtime:

```bash
ctest --test-dir build -R "codegen|pipeline" --output-on-failure
ls build/teste-codegen-saida/*.py build/teste-pipeline-saida/*.py
```

## Scanner e inspeção de tokens (OX-04)

```bash
ctest --test-dir build -R lexer --output-on-failure
./build/oxente-tokens examples/valid/calculo.oxe
printf 'arrodeia\r\n0' > build/tokens-valido.oxe
./build/oxente-tokens build/tokens-valido.oxe
# ARRODEIA 1:1-1:9 "arrodeia"
# NL 1:9-2:1 "\r\n"
# INTEIRO 2:1-2:2 "0"
# EOF 2:2-2:2 ""
# código 0
printf '\r\n\r\n@' > build/tokens-invalido.oxe
./build/oxente-tokens build/tokens-invalido.oxe
# stderr: build/tokens-invalido.oxe:3:1: erro léxico: caractere não reconhecido: "@"
# código 1; stdout contém os NL anteriores ao erro
```

O executável separado aceita um caminho (sem opções); imprime nome do token,
intervalo `linha:coluna-linha:coluna` e lexema entre aspas, com caracteres de
controle escapados **na saída de inspeção**. Isso não define escapes da linguagem.
Códigos: 0 tokenização concluída, 1 erro léxico ou caso documental pendente,
2 uso incorreto, 3 falha de abertura/leitura. Tokenização concluída não é validação
sintática/semântica.

A integração conserva o esqueleto C do Bison e o scanner C do Flex compilados
como C++17, sem `libfl`, com prefixo `oxinfra`. O scanner agora é reentrante e o
parser usa `api.pure full`, `%locations` e o parâmetro `ParserContext*`.
`src/frontend/infra.y` é a fonte única dos tokens. `oxente::parse` constrói a AST
do núcleo da linguagem; entrada vazia ou só com linhas vazias é rejeitada.

`oxente::Scanner` em `src/frontend/scanner.hpp` encapsula o estado Flex e empresta
o `FILE*` do consumidor. Cada token não EOF entrega um `std::string*` em
`OXINFRASTYPE::texto`: **o consumidor deve liberá-lo**. No parser, as ações movem
ou liberam esses valores, e `proximo_significativo` libera os tokens que não
carregam valor. Todos os lexemas são preservados,
inclusive zeros iniciais, vírgula decimal e delimitadores de texto, sem converter
para `int`/`double` nem fixar limites numéricos ou espaços de borda do valor textual.

Localizações começam em 1 e usam fim exclusivo. Cada byte ASCII e cada tabulação
avançam uma coluna de origem; CRLF avança uma linha, como LF. EOF tem intervalo
vazio no cursor e `proximo` não cria `NL` artificial; `proximo_significativo`, usado
pelo parser, entrega uma quebra antes do EOF para encerrar a última instrução. `ErroLexico` carrega intervalo e mensagem; a análise
para no primeiro erro e o consumidor deve abandonar essa instância do scanner.

Como espaços significativos e o tratamento sintático de comentários permanecem
pendentes, o scanner preserva espaços/tabulações externos em `ESPACO` e emite
`NL` para **todas as quebras físicas fora de textos**, inclusive em comentários.
Não há tokens de indentação. O parser descarta `ESPACO` como limitação da
implementação atual; isso não resolve a regra documental de espaços significativos.
Comentário não gera outros tokens nem aninha; posicionamento entre instruções
completas será responsabilidade da integração futura, não validado pelo scanner.

Textos suportados são ASCII não vazios em uma linha, com exatamente um espaço ou
tabulação em cada borda; espaços internos são preservados. Texto vazio, múltiplos
espaços de borda, barra inversa/escapes, texto multilinha, CR isolado, comentário
de linha no EOF e números adjacentes a identificadores produzem diagnóstico
**caso pendente na especificação**, sem declará-los erros aprovados da linguagem.
Textos/comentários de bloco sem fechamento no EOF são erros léxicos. Os marcadores
são palavras completas: `arrodeia_extra`, `espia_extra` e `tendeu_extra` não se
subdividem. `tendeu`/`arriba` fora dos estados correspondentes têm tokens próprios;
sua utilização será decidida pelo parser. As reservadas da extensão
`aperreio`/`toma`/`sem_futuro` são reconhecidas, sem suporte a funções.
`liso`/`balaio` não são reservadas nesta implementação.

Os testes de lexer conferem tokens, lexemas, início/fim, reservadas, operadores,
LF/CRLF/EOF, comentários, textos e falhas. Também exercitam instâncias isoladas,
rejeição de entrada vazia ou incompleta pelo parser e códigos/canais do harness. As pendências
continuam em [tokens](docs/tokens.md#pontos-a-confirmar); a OX-04 não as resolve.

## Parser de expressões e comandos simples (OX-05)

`oxente::parse(Scanner&, DiagnosticEngine* = nullptr)`, declarado em
`src/frontend/scanner.hpp`, retorna `std::unique_ptr<ProgramNode>` no sucesso e
`nullptr` em erro de entrada, sem AST parcial. Quando fornecido, o motor recebe
um diagnóstico após a liberação da pilha do Bison, inclusive quando configurado
para lançar `DiagnosticException`; essa exceção é contida pela API. O scanner
bruto continua lançando `ErroLexico` para seus consumidores diretos.

O subconjunto aceita declarações inicializadas dos quatro tipos, atribuições,
`diga_ai(identificador)` e `desembucha(expressao; ...)`, além das expressões da
[gramática](docs/grammar.md#expressões-esboço-por-níveis). Comandos consecutivos
exigem LF ou CRLF; linhas vazias são aceitas e EOF termina o último comando.
Expressões não continuam em outra linha. Programa e bloco principal recebem a
posição do primeiro comando, ignorando linhas iniciais vazias. Entrada vazia
continua inválida. A OX-06 estende esse subconjunto com condicionais e laços;
funções continuam fora do núcleo.

A AST atual armazena números como `int` e `double`. A conversão preserva a vírgula
decimal na origem; literais que não cabem nessa representação produzem diagnóstico
léxico `literal_fora_da_representacao` na posição do literal. Esses são limites da
implementação, não faixas aprovadas para a linguagem. O sinal permanece um operador
unário: inclusive a magnitude de um literal negativo passa pela conversão antes
do sinal. As operações são representadas em nós, sem avaliação pelo parser.

Os testes `parser` e `scope` exercitam a API real, incluindo agrupamentos da AST,
términos, argumentos de saída, localização e falhas com pilha parcialmente
construída. O teste manual da AST também conserva o construtor de saída com um
argumento. Espaços significativos e comentários de bloco no meio de expressões
continuam pendentes; o estado da CLI está registrado na tabela de etapas acima.

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
├── .github/workflows/
├── docs/
│   ├── AGENT.md
│   ├── language-spec.md
│   ├── tokens.md
│   ├── grammar.md
│   ├── semantics.md
│   ├── architecture.md
│   ├── test-suite.md
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
as [regras semânticas](docs/semantics.md) e a [arquitetura](docs/architecture.md).
Os [exemplos e resultados esperados](examples/README.md) são referências manuais;
ainda não foram executados de ponta a ponta por um tradutor.

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
