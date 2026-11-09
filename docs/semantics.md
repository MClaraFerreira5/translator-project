# Regras semânticas

Base: §§2 e 5–6 da [definição entregue](references/definicao-entregue.md).
O analisador verifica o núcleo implementado; regras provisórias e extensões
futuras estão identificadas abaixo.
Consulte a [gramática](grammar.md) e os [casos de referência](../examples/README.md).

## API implementada

`oxente::SemanticAnalyzer`, em `src/semantic/SemanticAnalyzer.hpp`, recebe uma
AST por `no.accept(analisador)`. O visitante verifica nomes, tipos e contexto de
laços; não avalia expressões nem gera Python. Os tipos resolvidos ficam em
`ExprNode::tipo_calculado`. A CLI chama esse analisador em `--check` e
`--emit-python`; `--dump-ast` faz apenas a análise sintática.

Todos os argumentos de saída são percorridos na ordem da AST. Uma declaração
sem inicializador é rejeitada na posição da declaração com
`inicializador_ausente`, inclusive em ASTs manuais, sem inserir o nome na tabela.
O inicializador existente continua sendo analisado antes da inserção do nome.
Incompatibilidades de operadores usam `misturou_as_coisas` e informam a operação,
a exigência e os tipos encontrados; `TypeKind::Erro` permanece um sentinela
interno de propagação, sem substituir o tipo real nos diagnósticos dos operadores.

Com `DiagnosticEngine` no modo padrão, a falha lança `DiagnosticException`.
O consumidor deve tratar essa exceção como falha mesmo quando ela ocorre na
tabela de símbolos antes de `SemanticAnalyzer::tem_erros` ser marcada. Com
`parar_no_primeiro=false`, a análise pode continuar e o consumidor consulta
`tem_erros` e `motor.tem_erros()`. Sem motor, a flag do visitante sinaliza a
falha, sem emitir mensagem. Anotações de uma análise com erro não autorizam
geração. A reutilização do visitante após exceção ainda não tem contrato;
o consumidor deve descartá-lo nessa situação.

As matrizes pendentes de comparação, os detalhes de `arrodeia` e os metadados
adicionais para o gerador não são definidos por esta API. O gerador mantém seu
próprio ambiente de nomes e tipos ao percorrer a AST validada. Os testes `semantic`
usam ASTs manuais; as regressões de integração em `parser` exercitam também
`oxente::parse` seguido pelo analisador.

| Regra | Estado |
| --- | --- |
| Declaração antes do uso, inicializador antes do nome, duplicação, sombreamento e nome de bloco encerrado | Implementado em `SemanticAnalyzer` e `SymbolTable` |
| Tipos dos operadores, atribuições e condições `papo` | Implementado em `SemanticAnalyzer` |
| `canso`/`simbora` fora de laço | Implementado em `SemanticAnalyzer` (`contexto_invalido`) |
| Zero em `/`, `//`, `%`, domínio de `**` e entrada inválida ou ausente | Implementado no runtime, com código de erro e linha |
| Faixas numéricas | Faixa inteira pendente; conversões e resultados reais não finitos verificados no [runtime](runtime.md) |

## Declaração, inicialização e escopo

- Declarar antes do uso, com tipo explícito e inicializador obrigatório.
- Analisar o inicializador antes de disponibilizar o nome novo: `inteiro x bota
  x + 1` é inválido se `x` ainda não existir.
- O tipo é fixo. `bota` atualiza o valor de variável existente, sem redeclará-la.
- Cada bloco tem escopo próprio, com consulta do mais interno para o externo.
  Nomes de blocos encerrados deixam de ser visíveis.
- Rejeitar nome já visível, inclusive em bloco interno: não há sombreamento.
  Nomes podem ser reutilizados em blocos independentes após saírem de visibilidade.

A tabela de símbolos registra nome, tipo, escopo, localização da declaração e
estado de inicialização. A análise percorre a AST, controlando escopos e contexto
de laço. O gerador mantém seu próprio ambiente ao percorrer a mesma AST validada.

## Tipagem e operadores

| Construção | Tipos exigidos e resultado previsto |
| --- | --- |
| Declaração/atribuição | Mesmo tipo; admite somente promoção implícita de `inteiro` para `tronxo` |
| `+`, `-`, `*` binários | Numéricos; mistura promove `inteiro` para `tronxo`; entre inteiros preserva `inteiro` |
| `+`, `-` unários | Numéricos; preservam o tipo numérico |
| `/` | Numéricos; resultado `tronxo`, mesmo com dois inteiros |
| `//`, `%` | Apenas `inteiro`; resultado `inteiro` |
| `**` | Numéricos; resultado `tronxo` |
| `abs(...)` | Numérico; preserva o tipo do argumento |
| `<`, `<=`, `>`, `>=`, `==`, `!=` | Resultado `papo`; compatibilidade exata de operandos a confirmar |
| `ai_dento`, `e`, `ou` | Operandos do tipo `papo`; resultado `papo` |
| Condicionais e `ate` | Condição do tipo `papo`, sem conversão de número/texto |

As regras numéricas de promoção orientam a leitura da tabela; a matriz exata de
comparações (inclusive igualdade de textos/booleanos) não foi explicitada na
fonte. Os exemplos usam comparação entre inteiros. A implementação aceita, de forma
provisória, igualdade entre números ou valores do mesmo tipo, e ordenação só entre
números. Não inferir suporte a ordenar
textos ou comparar tipos distintos. Não há concatenação: `+` é numérico.

Não são aceitas conversões implícitas de `tronxo` para `inteiro`, de texto para
número ou de número para `papo`. `evera` e `pala` são os únicos literais booleanos.

## Fluxo e entrada/saída

`num_para` testa a condição de parada antes do corpo; com `evera` não entra e com
`pala` continua. O exemplo da fonte soma de 1 a 5 e para com contador igual a 6.
`arrodeia` também usa parada, mas seus detalhes estão pendentes na [gramática](grammar.md).

`canso` interrompe uma repetição e `simbora` pula uma iteração; fora de laços
constituem erro semântico. O gerador vincula ambos ao laço mais interno, executa
a atualização de `arrodeia` em `simbora` e não a executa em `canso`. A
[arquitetura](architecture.md#gerador-python-3-ox-11-e-ox-12) registra o que disso
é provisório.

`diga_ai` exige identificador declarado e entrada compatível com seu tipo.
`desembucha` aceita expressões em argumentos separados por `;`. Inteiros são
exibidos sem decimais; `tronxo`, com vírgula e três casas decimais por padrão.
A saída de referência `desembucha(10; vei pontos tendeu)` combina argumentos
sem concatenação. O runtime separa os argumentos com um espaço e termina com
quebra de linha, de forma provisória; separadores, término de saída e entrada de
cada tipo ainda precisam de especificação precisa.

## Diagnósticos e momento de detecção

| Momento | Erros e tratamento previstos |
| --- | --- |
| Léxico, antes da tradução | Caracteres inválidos, texto/comentário não terminado |
| Sintático, antes da tradução | Estruturas fora da gramática, operandos ou delimitadores ausentes |
| Semântico, antes da tradução | Nome não declarado/duplicado, tipos incompatíveis, condição não booleana, `canso`/`simbora` fora de laço |
| Execução do programa gerado | Divisão por zero, entrada inválida/ausente, domínio da potência e resultados fora das faixas numéricas válidas |

A análise para no primeiro erro, com diagnóstico e status de falha. Recuperação
de múltiplos erros é possibilidade futura. Mensagens devem indicar localização
(linha e coluna quando disponível) e informação suficiente para correção.

- Nome ausente: mensagem como `Tais bestando? Variavel quantidade nao declarada`,
  acrescida de localização.
- Incompatibilidade: `misturou_as_coisas`, com tipo esperado e encontrado.
- Nome duplicado: `valor_sem_pe_nem_cabeca`.
- Zero em `/`, `//` ou `%`: interromper com `dividiu_por_nada`, operação e linha
  de origem. O gerador chama helpers do runtime que fazem essa verificação, sem
  delegar o diagnóstico ao interpretador Python 3.

Operações só são válidas dentro das faixas dos tipos escolhidos. A implementação
deverá documentar limites e verificar operações para rejeitar valores fora da
faixa e resultados reais não finitos, mesmo que Python aceite inteiros maiores.
O runtime rejeita entradas reais não finitas, resultados não finitos de
`+`, `-`, `*`, `/` e `**`, e falhas de conversão de inteiro para real com
diagnóstico localizado. Consulte os códigos e limites em
[runtime](runtime.md). Larguras de tipos e detalhes de arredondamento ainda
não foram decididos. Não há vetores/indexação,
portanto acesso fora dos limites não integra esta versão.

## Exemplos de decisões

Aceito pela especificação, por promoção para `tronxo`:

```text
inteiro quantidade bota 3
tronxo total bota quantidade
total bota total + 0,5
```

Rejeitado por conversão implícita inversa:

```text
tronxo media bota 7,5
inteiro resultado bota media
```

Rejeitado por condição não booleana:

```text
vai_que (1) bora
    desembucha(vei Oi tendeu)
arremata
```

Os arquivos em [examples](../examples/README.md) isolam outras falhas e resultados
esperados. O teste `e2e` traduz e executa os válidos e confere o diagnóstico dos
inválidos.

## Extensão futura de funções

Funções terão nomes globais sem sobrecarga e sem colisão com identificadores
visíveis. Parâmetros não poderão se repetir; serão inicializados por argumentos.
Argumentos e retornos usarão a mesma compatibilidade de tipos, incluindo promoção
para `tronxo`. Serão verificadas chamadas, parâmetros e retornos quando essa
extensão existir. `toma` e `sem_futuro` não acrescentam instruções ao núcleo.

## Pontos a confirmar

- Matriz de tipos dos operadores relacionais e igualdade.
- Curto-circuito de `e`/`ou` e ordem de avaliação de operandos/argumentos.
- Regra de divisão inteira e resto para negativos: a OX-01 cita truncamento
  para zero como regra original, mas o §2.3 da cópia entregue não o explicita.
  O runtime trunca para zero; confirmar com o grupo. Confirmar também o domínio
  detalhado de `**`.
- Faixas numéricas, precisão, arredondamento para três casas e diagnósticos de
  overflow/resultado não finito. Não impor limites do C++ ou a precisão
  arbitrária dos inteiros Python como regra aprovada.
- Formatos de entrada (especialmente `papo`, `tronxo`, `resenha`) e separação
  exata na saída; a fonte dá exemplos, mas não um contrato completo de I/O.
- Escopo e atualização de `arrodeia`, controle em laços aninhados e política de
  retorno em todos os caminhos na extensão futura.
- `liso`/`balaio` permanecem sem gramática, tipo e comportamento, fora do núcleo.

O destino da geração é Python 3. As diferenças de escopo, operadores, avaliação
e entrada/saída, e como cada uma é tratada, estão nos
[riscos da tradução](architecture.md#riscos-da-tradução-para-python-3).
