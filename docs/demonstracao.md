# Demonstração da Oxente Lang

Roteiro principal sugerido: **10 a 12 minutos**, com três blocos de fala. Execute
os comandos na raiz do checkout, em Linux ou Ubuntu/WSL com as
[dependências instaladas](preparacao.md). Deixe o editor aberto em `infra.l`,
`infra.y`, `SemanticAnalyzer.hpp` e `PythonGenerator.cpp`.

## Preparação antes da fala

Em um checkout limpo, use uma pasta de build ainda inexistente. Se
`build/apresentacao` já existir, use outro nome e ajuste os comandos abaixo.
Compile uma vez antes da apresentação e mantenha as saídas nessa pasta ignorada
pelo Git. A instalação das dependências exige rede; a demonstração após o build
pode ser feita sem rede.

```bash
cmake -S . -B build/apresentacao -DBUILD_TESTING=ON
cmake --build build/apresentacao --parallel 2
ctest --test-dir build/apresentacao --output-on-failure
./build/apresentacao/oxente --help
```

## 1. Do programa ao Python — integrante 1, cerca de 3 minutos

Apresente a proposta: uma linguagem própria, com tipagem estática e vocabulário
regional; C++17 implementa o tradutor, Flex reconhece tokens, Bison constrói a AST
e Python 3 executa o resultado. Abra [calculo.oxe](../examples/valid/calculo.oxe).

```bash
cat examples/valid/calculo.oxe
./build/apresentacao/oxente-tokens examples/valid/calculo.oxe
./build/apresentacao/oxente --dump-ast examples/valid/calculo.oxe
./build/apresentacao/oxente --check examples/valid/calculo.oxe
./build/apresentacao/oxente --emit-python examples/valid/calculo.oxe -o build/apresentacao/calculo.py
python3 build/apresentacao/calculo.py
```

Mostre `T_INTEIRO`, `IDENT`, `BOTA`, os operadores e suas localizações na listagem.
No dump, a multiplicação é filha da soma. A saída de `--check` termina em
`programa valido`. O Python executa e imprime:

```text
Resultado 14
Com parenteses 20
```

Explique a diferença: **`--dump-ast` mostra a árvore do parser; `--check` também
executa a análise semântica**. Tokenizar não prova validade sintática, e construir
a AST não prova validade semântica. O `.py` inclui o runtime; pode rodar sozinho.
Transição: “Vamos seguir uma dessas expressões pelo código e explicar onde cada
etapa toma sua decisão.”

## 2. Navegação pelo código — integrante 2, cerca de 4 minutos

Use uma instrução pequena para que a árvore inteira caiba na tela:

```bash
cat > build/apresentacao/percurso.oxe <<'OXE'
inteiro x bota 2 + 3 * 4
OXE
./build/apresentacao/oxente-tokens build/apresentacao/percurso.oxe
./build/apresentacao/oxente --dump-ast build/apresentacao/percurso.oxe
./build/apresentacao/oxente --check build/apresentacao/percurso.oxe
./build/apresentacao/oxente --emit-python build/apresentacao/percurso.oxe -o build/apresentacao/percurso.py
tail -n 5 build/apresentacao/percurso.py
python3 build/apresentacao/percurso.py
```

Esse programa só declara `x`; a execução não imprime nada. Mostre o trecho gerado
`v1_x = 2 + 3 * 4` no fim do arquivo, após o runtime. Percurso pesquisável:

| Etapa | Caminho relativo e ponto para buscar | O que explicar |
| --- | --- | --- |
| Flex | `src/frontend/infra.l`: `<INITIAL>"+"`, `EMITIR` | `+` retorna `MAIS`, guarda o lexema e recebe o intervalo de `YY_USER_ACTION`. Na linha do exemplo: `MAIS 1:18-1:19 "+"` |
| Palavra reservada | `src/frontend/infra.l`: `palavra`, `<INITIAL>{ID}` | `inteiro` consulta a tabela e vira `T_INTEIRO`; `x` vira `IDENT` |
| Adaptador | `src/frontend/scanner.cpp`: `Scanner::proximo_significativo` | Descarta `ESPACO`, libera lexemas sem valor semântico, conserva valores necessários e fornece `NL` no EOF se faltar quebra final |
| Interface Bison | `src/frontend/infra.y`: `%token MAIS`, `oxinfralex` | O parser usa o mesmo token `MAIS`; a função de integração chama o scanner significativo |
| Declaração | `infra.y`: `declaracao: tipo IDENT BOTA expressao`, `tipo` | `T_INTEIRO` vira `TypeKind::Inteiro`; a ação cria `VarDeclStmtNode` e transfere a expressão para `unique_ptr` |
| Precedência | `infra.y`: `expressao`, `disjuncao`, `conjuncao`, `negacao`, `comparacao`, `soma`, `produto`, `unario`, `potencia`, `primario` | A soma usa `soma MAIS produto`. O produto usa `produto VEZES unario`, por isso `3 * 4` forma um filho antes de a soma formar seu nó |
| Nós explícitos | `src/ast/StmtNodes.hpp`, `src/ast/ExprNodes.hpp` | `VarDeclStmtNode`, `BinaryExprNode` e `LiteralExprNode` guardam filhos e localização; o código do Bison não é a AST |
| Dump | `src/ast/AstPrinter.cpp`: `visit(VarDeclStmtNode`, `visit(BinaryExprNode` | Um visitante imprime a estrutura abaixo |
| Semântica | `src/semantic/SemanticAnalyzer.hpp`: `visit(VarDeclStmtNode`, `visit(BinaryExprNode`; `src/semantic/SymbolTable.hpp` | Analisa o inicializador antes de declarar `x`, resolve os operadores como `inteiro`, verifica compatibilidade e insere o nome no escopo |
| Python | `src/codegen/PythonGenerator.cpp`: `visit(VarDeclStmtNode`, `visit(BinaryExprNode` | Percorre a AST validada, preserva agrupamento e renomeia `x` como `v1_x` |
| Coordenação | `src/main.cpp`: `oxente::parse`, `SemanticAnalyzer`, `PythonGenerator` | A mesma AST segue pelas etapas. Um erro impede gerar Python |

Dump esperado:

```text
Programa
  Bloco
    Declaracao inteiro x
      Binario +
        Literal inteiro 2
        Binario *
          Literal inteiro 3
          Literal inteiro 4
```

Transição: “Agora vamos escrever um programa novo e mostrar o que acontece
quando uma entrada falha em cada etapa.”

## 3. Programa ao vivo e diagnósticos — integrante 3, cerca de 4 minutos

Escreva o programa abaixo do zero no editor, salvando em
`build/apresentacao/ao-vivo.oxe`. Para reproduzir a edição no terminal:

```bash
cat > build/apresentacao/ao-vivo.oxe <<'OXE'
inteiro numero bota 1
num_para ate (numero > 10) bora
    desembucha(numero)
    numero bota numero + 1
arremata
OXE
./build/apresentacao/oxente --check build/apresentacao/ao-vivo.oxe
./build/apresentacao/oxente --emit-python build/apresentacao/ao-vivo.oxe -o build/apresentacao/ao-vivo.py
python3 build/apresentacao/ao-vivo.py
```

`ate` é **condição de parada**: enquanto `numero > 10` for `pala`, execute o corpo.
Ao chegar a 11, pare antes de imprimir. A saída completa é:

```text
1
2
3
4
5
6
7
8
9
10
```

O arquivo de exercício fica no build; não é um novo exemplo do catálogo.
Para conferir automaticamente as dez linhas após executar:

```bash
python3 build/apresentacao/ao-vivo.py > build/apresentacao/ao-vivo.txt
python3 - <<'PY'
from pathlib import Path
esperado = ''.join(f'{n}\n' for n in range(1, 11))
assert Path('build/apresentacao/ao-vivo.txt').read_text() == esperado
print('Saida de 1 a 10 conferida.')
PY
```

Em seguida, rode um caso por vez; cada um deve retornar **1**, emitir diagnóstico
em **stderr** e deixar **stdout vazio**. O `printf` exibe o código do comando
anterior; não execute outro comando entre eles.

```bash
./build/apresentacao/oxente --check examples/invalid/lexico-caractere.oxe
printf 'Codigo: %s\n' "$?"
./build/apresentacao/oxente --check examples/invalid/sintatico-operando.oxe
printf 'Codigo: %s\n' "$?"
./build/apresentacao/oxente --check examples/invalid/semantico-nao-declarado.oxe
printf 'Codigo: %s\n' "$?"
```

| Caso isolado | Indicação no diagnóstico |
| --- | --- |
| Léxico | `[léxico]`, `:1:22:`, `erro_lexico` e caractere `@` não reconhecido |
| Sintático | `[sintático]`, `:1:24:`, `sintaxe_invalida`; falta o operando depois de `+` antes do fim da linha |
| Semântico | `[semântico]`, `:1:24:`, `nome_nao_declarado`; nome `ausente` não declarado |

A mensagem sintática atual é genérica (`Entrada fora da gramatica.`); explique a
causa mostrando a linha do exemplo, sem prometer uma mensagem mais específica.

## Trechos opcionais e perguntas cruzadas

Se houver tempo, execute os exemplos existentes com os mesmos comandos de geração:
[decisao.oxe](../examples/valid/decisao.oxe) para condições,
[repeticao.oxe](../examples/valid/repeticao.oxe) para acumulação e
[reais.oxe](../examples/valid/reais.oxe) para vírgula decimal. O
[índice](../examples/README.md) informa as saídas. Para questões de implementação,
abra [arquitetura](architecture.md), [frontend](frontend.md) ou [runtime](runtime.md).

- Integrante 1 pergunta ao 2: por que a multiplicação aparece abaixo da soma na AST?
- Integrante 2 pergunta ao 3: por que `--dump-ast` aceita uma variável não declarada,
  mas `--check` rejeita? Onde o nome passa a existir na tabela de símbolos?
- Integrante 3 pergunta ao 1: o que ocorre se `ate` já for `evera` no primeiro teste?
  Como `simbora` em `arrodeia` preserva a atualização?
- Todos: quem libera os lexemas? Por que o Python precisa de helpers para divisão,
  saída de `tronxo` e diagnósticos? O que é provisório e o que não é suportado?

A distribuição dos nomes e a duração são sugestões; todos devem ensaiar o pipeline
inteiro e trocar os blocos para responder perguntas fora de sua parte.

## Lista de checagem

Verificações locais desta preparação (evidências no [registro](revisao-apresentacao.md)):

- [x] Configurar e compilar em pasta nova com Flex/Bison, sem conflitos novos.
- [x] Executar CTest completo, incluindo catálogo e ponta a ponta.
- [x] Executar tokens, dump, análise, geração e Python do caminho principal.
- [x] Escrever e executar o programa de 1 a 10; comparar as dez linhas.
- [x] Conferir os três diagnósticos e seus códigos/canais.

Confirmações que dependem do grupo:

- [ ] Testar dependências e comandos na máquina da apresentação (inclusive WSL, se usado).
- [ ] Conferir versão do checkout, fonte do terminal, zoom do editor e projeção.
- [ ] Fazer o build antes da fala e deixar arquivos de exemplo acessíveis sem rede.
- [ ] Definir quem apresenta cada bloco e cronometrar o ensaio coletivo.
- [ ] Cada integrante escrever o exercício sem copiar e explicar o mapa completo.
- [ ] Ensaiar perguntas cruzadas, erros e recuperação de uma edição incorreta.
- [ ] Consolidar a declaração de ferramentas de IA e atividades usadas pelo grupo.
- [ ] Confirmar com o grupo/professor as regras provisórias registradas na documentação.
