# Gramática inicial e abordagem sintática

Base: §§2–5 da [definição entregue](references/definicao-entregue.md).
Esta EBNF é um **esboço documental**, não uma gramática Bison implementada,
completa ou comprovadamente livre de conflitos. Os [tokens](tokens.md) e a
[semântica](semantics.md) complementam as regras de estrutura.

## Notação e limites

Aspas representam grafias de tokens; `|` indica alternativa, `[ ]` opcionalidade
e `{ }` repetição. `IDENT`, `INTEIRO`, `REAL`, `TEXTO` e `FIM_LINHA` são categorias
descritivas, sem compromisso com nomes futuros no Bison. `TEXTO` representa o
literal inteiro delimitado por `vei`/`tendeu`.

No esboço, LF e CRLF correspondem a `FIM_LINHA`; EOF pode fornecer o término
da última instrução. Comentários são tratados pelo léxico, mantendo localização
e términos significativos conforme [tokens](tokens.md). A estratégia exata
lexer/parser, inclusive EOF após comentários e linhas vazias, ainda será definida.

## Estruturas amparadas na fonte

```ebnf
programa       = { FIM_LINHA }, instrucao, { FIM_LINHA | instrucao } ;
sequencia      = { FIM_LINHA | instrucao } ;
instrucao      = simples, FIM_LINHA | condicional | enquanto | para ;
simples        = declaracao | atribuicao | entrada | saida
               | "canso" | "simbora" ;
tipo           = "inteiro" | "tronxo" | "papo" | "resenha" ;
declaracao     = tipo, IDENT, "bota", expressao ;
atribuicao     = IDENT, "bota", expressao ;
entrada        = "diga_ai", "(", IDENT, ")" ;
saida          = "desembucha", "(", argumentos, ")" ;
argumentos     = expressao, { ";", expressao } ;
bloco          = "bora", FIM_LINHA, sequencia, "arremata", FIM_LINHA ;
condicional    = "vai_que", "(", expressao, ")", bloco,
                 { "se_num_for", "(", expressao, ")", bloco },
                 [ "pois_pronto", bloco ] ;
enquanto       = "num_para", "ate", "(", expressao, ")", bloco ;
```

Declarações exigem inicializador; `bota` não pertence às expressões.
`desembucha` com pelo menos um argumento é a forma ilustrada; a possibilidade
de chamada vazia não é decidida pela proposta. `sequencia` permite bloco vazio
neste esboço como hipótese a confirmar, sem afirmar sua aceitação na linguagem.

Condições são verificadas semanticamente como `papo`. `canso` e `simbora` são
representados como instruções simples, ficando a restrição de contexto para a
análise semântica. `se_num_for`/`pois_pronto` só completam um `vai_que`.
Após `bora` e `arremata` há término de linha; cada fechamento corresponde à
abertura mais próxima. O esboço não resolve ainda linhas vazias/comentários entre
ramos de um mesmo condicional, e não autoriza cabeçalhos partidos em várias linhas.

## Esboço pendente de `arrodeia`

A tabela final fornece `arrodeia (...; ate ...; ...)`; o corpo prevê
inicialização, parada e atualização. Uma concretização **a confirmar** seria:

```ebnf
para           = "arrodeia", "(", inicio_para, ";", "ate", expressao,
                 ";", atribuicao, ")", bloco ;
inicio_para    = declaracao | atribuicao ;
```

Não se considera aprovada a alternativa declaração/atribuição, a obrigatoriedade
de cada campo ou a ausência de parênteses adicionais na condição. Também é
necessário confirmar escopo do inicializador e destino de `simbora` na atualização.
Não há exemplo classificado como válido para essa forma enquanto essas decisões
estiverem abertas. `ate` expressa parada, e não condição para continuar.

## Expressões: esboço por níveis

```ebnf
expressao      = disjuncao ;
disjuncao      = conjuncao, { "ou", conjuncao } ;
conjuncao      = negacao, { "e", negacao } ;
negacao        = "ai_dento", negacao | comparacao ;
comparacao     = soma, [ relacional, soma ] ;
relacional     = "<" | "<=" | ">" | ">=" | "==" | "!=" ;
soma           = produto, { ("+" | "-"), produto } ;
produto        = unario, { ("*" | "/" | "//" | "%"), unario } ;
unario         = ("+" | "-"), unario | potencia ;
potencia       = primario, [ "**", unario ] ;
primario       = INTEIRO | REAL | "evera" | "pala" | TEXTO | IDENT
               | "(", expressao, ")" | "abs", "(", expressao, ")" ;
```

As repetições de operadores binários serão agrupadas à esquerda, não apenas
armazenadas em uma lista sem associatividade. A recursão de `**` agrupa à direita.
O lado direito unário de `**` permite expoente com sinal neste esboço; essa
combinação específica merece confirmação, pois não aparece nos exemplos da fonte.
Chamadas de funções de usuário ficam exclusivamente na extensão futura.

| Prioridade (maior primeiro) | Operadores/construção | Associatividade da fonte |
| --- | --- | --- |
| 1 | Parênteses e chamadas | Agrupamento explícito |
| 2 | `**` | Direita |
| 3 | `+`, `-` unários | Direita |
| 4 | `*`, `/`, `//`, `%` | Esquerda |
| 5 | `+`, `-` binários | Esquerda |
| 6 | `<`, `<=`, `>`, `>=`, `==`, `!=` | Sem encadeamento |
| 7 | `ai_dento` | Direita |
| 8 | `e` | Esquerda |
| 9 | `ou` | Esquerda |

Consequências previstas: `2 + 3 * 4` vale 14; `(2 + 3) * 4` vale 20;
`10 - 3 - 2` vale 5; `2 ** 3 ** 2` vale 512 como `tronxo`;
`-2 ** 2` vale -4 como `tronxo`. `a < b < c` é rejeitado como comparação
encadeada. `ai_dento a == b` agrupa a comparação antes da negação.

## Funções e procedimentos: extensão futura

A grafia canônica é `aperreio`, embora o §2.6 ainda diga `funcao`.
Parâmetros terão tipo e nome, separados por `;`; `toma expressao` retornará um
valor compatível. `sem_futuro` é a equivalência de procedimento sem valor de
retorno na tabela final, sem sintaxe completa definida.

Definições ficarão no nível superior. Chamadas com retorno poderão integrar
expressões; chamadas também poderão ser instruções. Não há `main` obrigatório.
Não se fixa uma produção de declaração de função: posição do tipo de retorno,
forma de procedimento, retorno vazio e demais detalhes dependem de confirmação.

## Pontos a confirmar

- Formalizar campos e escopo de `arrodeia`, além da interação com `simbora`.
- Confirmar blocos vazios, chamada de saída vazia e separação entre ramos por
  linhas vazias/comentários.
- Resolver espaços significativos e textos antes da integração lexer/parser.
- Confirmar potência com expoente unário e explicitar casos de combinação de
  operadores que o esboço permite, sem apresentá-los como já testados.
- Completar sintaxe da extensão e revisar `funcao`/`aperreio` na fonte.

A etapa futura com Bison deverá verificar conflitos e adequar o esboço às
decisões confirmadas. Nenhum `parser.y` foi criado nesta etapa.
