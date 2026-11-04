# Oxente Lang

Linguagem educacional do projeto da disciplina de Compiladores. A modalidade
escolhida é um **tradutor Oxente Lang → Python 3**, com implementação planejada em
**C++17**, usando **Flex** para análise léxica e **Bison** para análise sintática.

**Estado atual (OX-04):** especificação aprovada, build CMake, CLI e scanner Flex
com testes independentes da gramática. O scanner reconhece o subconjunto léxico
inequívoco e sinaliza limitações documentais. Parser funcional, AST, semântica e
tradução ainda não estão disponíveis.

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
sintática/semântica. A CLI `oxente` mantém os três modos não implementados.

A integração conserva o esqueleto C do Bison e o scanner C do Flex compilados
como C++17, sem `libfl`, com prefixo `oxinfra`. O scanner agora é reentrante e o
parser usa `api.pure full`, `%locations` e o parâmetro `void* scanner`.
`src/frontend/infra.y` é a fonte única dos tokens. O parser provisório continua
rejeitando qualquer entrada, inclusive vazia; não tem produções da linguagem.

`oxente::Scanner` em `src/frontend/scanner.hpp` encapsula o estado Flex e empresta
o `FILE*` do consumidor. Cada token não EOF entrega um `std::string*` em
`OXINFRASTYPE::texto`: **o consumidor deve liberá-lo**. O Bison já declara
`%destructor`; futuras ações da OX-05/OX-07 deverão mover/copiar esses valores e
liberá-los ao construir a AST explícita. Todos os lexemas são preservados,
inclusive zeros iniciais, vírgula decimal e delimitadores de texto, sem converter
para `int`/`double` nem fixar limites numéricos ou espaços de borda do valor textual.

Localizações começam em 1 e usam fim exclusivo. Cada byte ASCII e cada tabulação
avançam uma coluna de origem; CRLF avança uma linha, como LF. EOF tem intervalo
vazio no cursor e não cria `NL` artificial: o parser futuro deverá consumi-lo como
término da última instrução. `ErroLexico` carrega intervalo e mensagem; a análise
para no primeiro erro e o consumidor deve abandonar essa instância do scanner.

Como espaços significativos e o tratamento sintático de comentários permanecem
pendentes, o scanner preserva espaços/tabulações externos em `ESPACO` e emite
`NL` para **todas as quebras físicas fora de textos**, inclusive em comentários.
Não há tokens de indentação nem decisão de como esses tokens entram na gramática.
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
rejeição pelo parser provisório e códigos/canais do harness. As pendências
continuam em [tokens](docs/tokens.md#pontos-a-confirmar); a OX-04 não as resolve.

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
