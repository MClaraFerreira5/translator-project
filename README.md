# Oxente Lang

Oxente Lang é uma linguagem educacional com vocabulário inspirado em expressões
nordestinas, criada para o projeto da disciplina de Compiladores. Seu tradutor
usa **C++17, Flex e Bison** para analisar programas `.oxe`, construir uma AST
explícita, verificar nomes e tipos e gerar **Python 3**. Python simplifica a geração
e permite executar o resultado sem compilar a linguagem de destino.

O núcleo implementa quatro tipos (`inteiro`, `tronxo`, `papo`, `resenha`),
expressões, variáveis, entrada/saída, condicionais, laços e escopos por bloco.

Este trecho de [calculo.oxe](examples/valid/calculo.oxe) mostra a precedência:

```text
inteiro primeiro bota 2
inteiro segundo bota 3
inteiro terceiro bota 4
inteiro resultado bota primeiro + segundo * terceiro
inteiro agrupado bota (primeiro + segundo) * terceiro
desembucha(vei Resultado tendeu; resultado)
desembucha(vei Com parenteses tendeu; agrupado)
```

Saída:

```text
Resultado 14
Com parenteses 20
```

## Início rápido

No Debian/Ubuntu, instale as dependências:

```bash
sudo apt-get update
sudo apt-get install g++ cmake make flex bison m4 python3
```

Na raiz deste repositório, compile, teste, valide, traduza e execute:

```bash
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
./build/oxente --check examples/valid/calculo.oxe
./build/oxente --emit-python examples/valid/calculo.oxe -o build/calculo.py
python3 build/calculo.py
```

O Python gerado contém o runtime e pode ser executado sozinho. Para Fedora,
pré-requisitos detalhados e Windows com WSL, consulte a [preparação](docs/preparacao.md).
O comando `sh scripts/build-e-testes.sh` também compila e executa o CTest.

## Explorar o tradutor

| Comando | O que mostra |
| --- | --- |
| `./build/oxente --check arquivo.oxe` | Análises léxica, sintática e semântica |
| `./build/oxente --dump-ast arquivo.oxe` | Árvore construída pelo parser, antes da análise semântica |
| `./build/oxente --emit-python arquivo.oxe` | Python em stdout após validar o programa |
| `./build/oxente --emit-python arquivo.oxe -o build/saida.py` | Python escrito no caminho indicado |
| `./build/oxente-tokens arquivo.oxe` | Tokens, lexemas e localizações; executável produzido pelo mesmo build |

Use `./build/oxente --help` para consultar o uso. O [contrato da CLI](docs/cli.md)
detalha argumentos, diagnósticos e códigos de saída.

## Documentação e exemplos

O [índice de documentação](docs/README.md) reúne os guias técnicos.

- [Demonstração ao vivo](docs/demonstracao.md): comandos, exercício e percurso pelo código.
- [Linguagem](docs/language-spec.md), [tokens](docs/tokens.md), [gramática](docs/grammar.md) e [semântica](docs/semantics.md).
- [Arquitetura](docs/architecture.md): pipeline, AST, visitantes e geração Python.
- [Exemplos e resultados esperados](examples/README.md) e [testes](docs/test-suite.md).
- [Fontes entregues e procedência](docs/references/README.md).

## Limites atuais

Funções e procedimentos são extensões futuras; `liso`, `balaio` e vetores não são
suportados. Algumas regras da proposta ainda precisam de confirmação do grupo,
como detalhes de `arrodeia`, textos e entrada/saída. A implementação provisória
está identificada nos guias da linguagem.

Literais inteiros precisam caber no `int` de C++; a faixa inteira durante a
execução ainda não é verificada. O [runtime](docs/runtime.md) documenta as proteções
numéricas existentes. A rota documentada para Windows é WSL; há uma falha conhecida
do teste `lexer_harness` em build nativo com MSYS2.

## Equipe e uso de IA

- Adriano Júnior
- Maria Clara Ferreira
- Yann Leão

A disciplina permite apoio de IA. Os integrantes devem revisar e compreender o
material, explicar as decisões e declarar ferramentas e atividades utilizadas.
Registro consolidado de ferramentas e atividades: **a preencher pelo grupo**.

Para colaborar, leia [AGENTS.md](AGENTS.md) e [docs/AGENT.md](docs/AGENT.md).
Alterações ficam locais para revisão; edição não autoriza staging, commits,
pushes, merges, abertura de PR/MR ou publicação.
