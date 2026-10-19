# Catálogo de tokens e decisões léxicas

Base: §3 e tabela final da [definição entregue](references/definicao-entregue.md).
Este catálogo não é um lexer implementado. Consulte a [especificação](language-spec.md)
e a [gramática inicial](grammar.md).

## Caracteres, nomes e palavras

O código-fonte usa ASCII: letras, algarismos, sublinhado, espaços, tabulações,
quebras de linha e sinais necessários às construções. Maiúsculas e minúsculas
são distintas: `idade`, `Idade` e `IDADE` são nomes diferentes.

Identificador: `[A-Za-z_][A-Za-z0-9_]*`. Exemplos: `idade`, `_total`, `valor2`.
`2valor` não é um identificador válido; `nome-completo` também não forma um único
identificador, pois `-` é operador. A decomposição de sequências como `2valor`
em tokens e seu diagnóstico exato ainda precisam de decisão.

| Categoria | Grafias canônicas reservadas |
| --- | --- |
| Tipos | `inteiro`, `tronxo`, `papo`, `resenha` |
| Atribuição e blocos | `bota`, `bora`, `arremata` |
| Condicionais | `vai_que`, `se_num_for`, `pois_pronto` |
| Repetição | `num_para`, `arrodeia`, `ate`, `canso`, `simbora` |
| Valores e lógica | `evera`, `pala`, `ai_dento`, `e`, `ou` |
| Entrada, saída e cálculo | `diga_ai`, `desembucha`, `abs` |
| Texto e comentários | `vei`, `tendeu`, `primeiramente`, `espia`, `arriba` |
| Extensão futura de funções | `aperreio`, `toma`, `sem_futuro` |

As grafias são exatas e minúsculas. `Arrodeia` não é a palavra reservada de
repetição. Palavras da extensão constam na tabela de reservadas da proposta,
mas isso não implica suporte a declarações de função no núcleo. A estratégia
de habilitação da extensão ainda será decidida.

## Literais

| Categoria | Regra da proposta | Exemplos |
| --- | --- | --- |
| Inteiro | `[0-9]+` | `0`, `18`, `87` |
| Real (`tronxo`) | `[0-9]+,[0-9]+` | `7,5`, `3,000` |
| Booleano (`papo`) | `evera` ou `pala` | `evera` |
| Texto (`resenha`) | `vei` + espaço/tabulação + conteúdo + espaço/tabulação + `tendeu` | `vei Ola mundo tendeu` |

Sinais `+` e `-` são operadores unários, não parte das expressões regulares dos
literais numéricos. Não há forma exponencial nem decimal com ponto especificada.
As três casas decimais são uma regra de **exibição**, não uma restrição ao
número de casas em literais. Limites de representação ficam para a implementação.

Os textos não usam aspas. Palavras de comentário dentro deles não iniciam
comentários. Escape, texto vazio, multilinha, preservação de espaços de borda e
representação literal da palavra `tendeu` ainda não estão definidos.

## Operadores, delimitadores e pontuação

| Categoria | Tokens e função |
| --- | --- |
| Aritméticos | `+`, `-`, `*`, `/`, `//`, `%`, `**` |
| Relacionais | `>`, `<`, `>=`, `<=`, `==`, `!=` |
| Lógicos | `ai_dento`, `e`, `ou` |
| Atribuição | `bota`; `==` é comparação |
| Agrupamento e cabeçalhos/chamadas | `(`, `)` |
| Separador | `;` entre argumentos, parâmetros futuros e campos de `arrodeia` |
| Vírgula | Integra o literal real; não separa argumentos |
| Blocos | `bora`, `arremata`; não usar chaves |
| Término | Quebra de linha significativa; EOF termina a última instrução |

`abs` é palavra reservada de cálculo, com forma `abs(expressao)`.
`//` é divisão inteira, não comentário. `=` isolado e aspas não têm token de
atribuição/texto definido. Ao implementar, será necessário distinguir operadores
de vários caracteres de seus prefixos e palavras completas de identificadores;
as regras operacionais de reconhecimento ainda não foram testadas no Flex.

## Comentários e linhas

| Forma | Regra explícita |
| --- | --- |
| `primeiramente ...` | Até a quebra de linha; pode seguir uma instrução |
| `espia ... arriba` | Multilinha, entre instruções completas, sem aninhamento |

A quebra que encerra comentário de linha continua significativa. A contagem de
linhas deve ser mantida também dentro de comentários multilinha. Texto ou
comentário não terminado impede a tradução. Comentário de linha no EOF e
multilinha entre dois tokens na mesma linha precisam de detalhamento operacional.

LF e CRLF são aceitos e representam términos de linha. Linhas vazias são
permitidas. EOF tem efeito de término da última linha, mesmo sem LF final.
Não se permite continuação de expressões entre linhas. Comentários e linhas
vazias isolados não formam programa executável.

## Pontos a confirmar

- A fonte declara espaços em branco significativos, mas mostra indentação e
  define blocos por palavras. Confirmar se espaços/tabulações fora de textos
  apenas separam tokens. Não criar tokens de indentação por inferência.
- Confirmar limites lexicais de delimitadores de texto, regras para conteúdo
  vazio, escapes, novas linhas e espaços preservados. Os exemplos usam conteúdo
  ASCII simples, em uma linha, delimitado por espaços.
- Definir o tratamento de novas linhas internas a comentários multilinha na
  interface lexer/parser, preservando linhas sem dividir instruções incompletas.
- `liso` e `balaio` aparecem apenas no quadro final de equivalências: nem sua
  reserva lexical nem tipo, gramática ou comportamento estão definidos.
- As tabelas prevalecem sobre os remanescentes `funcao`, `Arrodeia`, `int`,
  `float`, `bool` e `string`. Não acrescentar essas grafias como aliases.
  `devolve` e `void` também não são palavras canônicas da origem.
- Confirmar diagnósticos para adjacências como `2valor`, tratamento de CR isolado
  e formas malformadas de números. Não apresentar uma estratégia como aprovada.
