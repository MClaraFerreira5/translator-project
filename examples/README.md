# Exemplos para validação futura

Os arquivos `.oxe` são referências documentais, classificados por leitura da
[especificação](../docs/language-spec.md). **Não foram executados ou validados
automaticamente por um tradutor Oxente Lang**, pois ele ainda não existe.
Os três primeiros casos válidos derivam do §7 da
[fonte entregue](../docs/references/definicao-entregue.md), com tabulações verticais
substituídas por quebras de linha LF reais somente nestas cópias derivadas.

Todos os códigos são ASCII, usam palavras canônicas e mantêm uma instrução por
linha. `valid/` e `invalid/` indicam expectativas para testes futuros.

## Casos válidos

| Arquivo | O que exercita | Resultado esperado |
| --- | --- | --- |
| [calculo.oxe](valid/calculo.oxe) | Multiplicação antes de soma e agrupamento por parênteses | Valores 14 e 20, acompanhados de `Resultado` e `Com parenteses`, respectivamente |
| [decisao.oxe](valid/decisao.oxe) | Condição `papo`, `ou`, `e`, `ai_dento` e ramo restante | Exibe `Acesso permitido`; se `bloqueado` mudar para `evera`, exibe `Acesso negado` |
| [repeticao.oxe](valid/repeticao.oxe) | `num_para ate`, acumulação e atualização | Exibe 15; contador termina em 6, quando a condição de parada é `evera` |
| [reais.oxe](valid/reais.oxe) | Promoção de inteiro, vírgula decimal e divisão real | Exibe os valores `3,500` e `2,500`, nessa ordem, com três casas decimais |
| [ramos.oxe](valid/ramos.oxe) | `se_num_for` com bloco próprio | Exibe `Recuperacao` |
| [comentarios.oxe](valid/comentarios.oxe) | Comentários de linha/multilinha e seus marcadores dentro de texto | Exibe o texto `primeiramente espia arriba` |
| [precedencia-potencia.oxe](valid/precedencia-potencia.oxe)	| Precedência de **, associatividade à direita e unário |	Calcula 2 ** 3 ** 2 == 512,000, -2 ** 2 == -4,000 e 3 * 2 ** 3 == 24,000
| [precedencia-logica.oxe](valid/precedencia-logica.oxe)	| Precedência ai_dento > e > ou e relacionais |	Avalia expressões booleanas compostas sem ambiguidades
| [operadores-longos.oxe](valid/operadores-longos.oxe)	| Operadores >=, <=, ==, !=, //, **, ai_dento, se_num_for, pois_pronto |	Reconhecimento e execução do fluxo condicional correto
| [nl-crlf-eof.oxe](valid/nl-crlf-eof.oxe)	| Múltiplas linhas vazias, comentários e término no EOF |	Reconhecimento correto de quebras de linha e separação de instruções
| [aninhamento.oxe](valid/aninhamento.oxe)	| Condicionais vai_que / se_num_for aninhados em laço num_para	| Execução estruturada de múltiplos níveis de blocos exibindo 32
| [escopo-independente.oxe](valid/escopo-independente.oxe)	| Reutilização de nomes em blocos irmãos independentes |	Variável local de bloco encerrado não interfere no bloco seguinte

A fonte ilustra o cálculo com `Resultado 14` e `Com parenteses 20` em linhas
distintas. Isso serve como referência, mas separação exata de argumentos e
término de saída ainda precisam de contrato explícito. Não se estabelece aqui
uma comparação de saída byte a byte. Não há exemplos válidos de `arrodeia`,
funções, `liso` ou `balaio`, cujas regras completas estão pendentes.

## Casos inválidos independentes

Cada arquivo isola um erro. A análise futura deve parar no primeiro erro,
emitir diagnóstico com localização e impedir a geração de código.

| Arquivo | Etapa esperada | Falha e diagnóstico esperado |
| --- | --- | --- |
| [lexico-caractere.oxe](invalid/lexico-caractere.oxe) | Léxica | Linha 1: `@` não possui token na linguagem; informar caractere inválido |
| [sintatico-operando.oxe](invalid/sintatico-operando.oxe) | Sintática | Linha 1: falta expressão após `+`, antes do término da linha |
| [semantico-nao-declarado.oxe](invalid/semantico-nao-declarado.oxe) | Semântica | Linha 1: nome `ausente` não declarado; mensagem de variável não declarada |
| [semantico-tipo.oxe](invalid/semantico-tipo.oxe) | Semântica | Linha 2: `misturou_as_coisas`, esperado `inteiro`, encontrado `tronxo` |
| [semantico-sombreamento.oxe](invalid/semantico-sombreamento.oxe) | Semântica | Linha 3: `valor_sem_pe_nem_cabeca`, nome `valor` já visível no bloco externo |
| [semantico-contexto.oxe](invalid/semantico-contexto.oxe) | Semântica | Linha 1: `canso` usado fora de laço |
| [lexico-texto-nao-terminado.oxe](invalid/lexico-texto-nao-terminado.oxe) | Léxica | Literal de texto iniciado por vei sem o fechamento tendeu |
| [lexico-comentario-nao-terminado.oxe](invalid/lexico-comentario-nao-terminado.oxe) | Léxica | Comentário de bloco espia sem delimitador arriba |
| [sintatico-parentese-desbalanceado.oxe](invalid/sintatico-parentese-desbalanceado.oxe) | Sintática | Falta fechar parêntese de condição vai_que (nota >= 5 |
| [sintatico-bloco-nao-fechado.oxe](invalid/sintatico-bloco-nao-fechado.oxe) | Sintática | Bloco bora sem o correspondente arremata |
| [sintatico-comparacao-encadeada.oxe](invalid/sintatico-comparacao-encadeada.oxe) | Sintática | Comparações encadeadas 1 < 2 < 3 proibidas pela gramática |
| [sintatico-tipo-sem-identificador.oxe](invalid/sintatico-tipo-sem-identificador.oxe) | Sintática | Declaração inteiro bota 5 sem identificador da variável |
| [semantico-uso-proprio-inicializador.oxe](invalid/semantico-uso-proprio-inicializador.oxe) | Semântica | Variável usada no próprio inicializador antes de declarada (x bota x + 1) |
| [semantico-operacao-tipo.oxe](invalid/semantico-operacao-tipo.oxe) | Semântica | Operação aritmética entre tipos incompatíveis (inteiro + resenha) |
| [semantico-duplicacao.oxe](invalid/semantico-duplicacao.oxe) | Semântica | Redeclaração da mesma variável no mesmo escopo |
| [semantico-escopo-fechado.oxe](invalid/semantico-escopo-fechado.oxe) | Semântica | Acesso a variável declarada em bloco já finalizado por arremata |
| [semantico-condicao-nao-booleana.oxe](invalid/semantico-condicao-nao-booleana.oxe) | Semântica | Condição de vai_que usando inteiro em vez de papo |
| [semantico-condicao-repeticao-nao-booleana.oxe](invalid/semantico-condicao-repeticao-nao-booleana.oxe) | Semântica | Condição de num_para ate usando inteiro em vez de papo |

As descrições expressam o conteúdo esperado do diagnóstico; não inventam códigos
ou redações obrigatórias onde a proposta não os define. Divisão por zero é erro
de execução, diferente desses erros que impedem a tradução; ver [semântica](../docs/semantics.md).

## Verificações futuras

Quando houver implementação real, incluir reconhecimento léxico, parsing e
verificação semântica desses casos, além de execução dos programas traduzidos.
Cobrir LF, CRLF e última linha sem quebra final; nesta etapa os arquivos de
exemplo usam LF e quebra final. Scripts de teste não são necessários na Sprint 0.
