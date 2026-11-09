# Scanner e parser: contratos de integração

## Inspeção de tokens

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
como C++17, sem `libfl`, com prefixo `oxinfra`. O scanner é reentrante e o
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
completas não é validado pelo scanner nem restringido explicitamente pelo parser; permanece uma divergência a confirmar.

Textos suportados são ASCII não vazios em uma linha, com exatamente um espaço ou
tabulação em cada borda; espaços internos são preservados. Texto vazio, múltiplos
espaços de borda, barra inversa/escapes, texto multilinha, CR isolado, comentário
de linha no EOF e números adjacentes a identificadores produzem diagnóstico
**caso pendente na especificação**, sem declará-los erros aprovados da linguagem.
Textos/comentários de bloco sem fechamento no EOF são erros léxicos. Os marcadores
são palavras completas: `arrodeia_extra`, `espia_extra` e `tendeu_extra` não se
subdividem. `tendeu`/`arriba` fora dos estados correspondentes têm tokens próprios;
o parser rejeita seu uso como instruções ou expressões. As reservadas da extensão
`aperreio`/`toma`/`sem_futuro` são reconhecidas, sem suporte a funções.
`liso`/`balaio` não são reservadas nesta implementação.

Os testes de lexer conferem tokens, lexemas, início/fim, reservadas, operadores,
LF/CRLF/EOF, comentários, textos e falhas. Também exercitam instâncias isoladas,
rejeição de entrada vazia ou incompleta pelo parser e códigos/canais do harness. As pendências
continuam em [tokens](tokens.md#pontos-a-confirmar); a implementação não as resolve.

## Parser e construção da AST

`oxente::parse(Scanner&, DiagnosticEngine* = nullptr)`, declarado em
`src/frontend/scanner.hpp`, retorna `std::unique_ptr<ProgramNode>` no sucesso e
`nullptr` em erro de entrada, sem AST parcial. Quando fornecido, o motor recebe
um diagnóstico após a liberação da pilha do Bison, inclusive quando configurado
para lançar `DiagnosticException`; essa exceção é contida pela API. O scanner
bruto continua lançando `ErroLexico` para seus consumidores diretos.

O parser aceita declarações inicializadas dos quatro tipos, atribuições,
`diga_ai(identificador)` e `desembucha(expressao; ...)`, além das expressões da
[gramática](grammar.md#expressões-esboço-por-níveis). Comandos consecutivos
exigem LF ou CRLF; linhas vazias são aceitas e EOF termina o último comando.
Expressões não continuam em outra linha. Programa e bloco principal recebem a
posição do primeiro comando, ignorando linhas iniciais vazias. Entrada vazia
continua inválida. Também aceita blocos, condicionais e laços; funções continuam fora do núcleo.

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
continuam pendentes; o contrato da CLI está em [cli.md](cli.md).
