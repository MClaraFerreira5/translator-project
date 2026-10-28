# Especificação consolidada da Oxente Lang

As regras desta consolidação derivam da [versão entregue](references/definicao-entregue.md).
A mudança de destino para Python 3 é uma decisão posterior explícita do grupo,
registrada na OX-01; a proposta histórica permanece preservada.
A proposta foi aprovada, mas o tradutor ainda não foi implementado. Regras
descritas aqui são compromissos documentais; esboços e lacunas são identificados.
Consulte a [procedência das fontes](references/README.md).

## Destino e escopo prioritário

O tradutor será implementado em **C++17 com Flex e Bison** e gerará **Python 3**.
O fluxo planejado é: arquivo `.oxe` → Flex → Bison → AST → análise
semântica/tabela de símbolos → gerador Python 3 → execução com Python 3.
A escolha segue a recomendação do professor para simplificar a geração de código
e dispensar a compilação da linguagem-alvo antes da execução.

O núcleo prioritário para **12/11/2026** inclui operações/expressões, os quatro
tipos, variáveis, atribuições, entrada/saída, condicionais, laços e seus controles,
AST, tabela de símbolos, verificações léxicas/sintáticas/semânticas e tradução
para Python 3. A entrega final está prevista para **26/11/2026**. Funções e
procedimentos são extensões após o núcleo; `liso` e `balaio` dependem de
especificação adicional. Nenhum desses recursos está implementado nesta etapa.

## Tipos, declarações e valores

| Tipo | Valores | Exemplo |
| --- | --- | --- |
| `inteiro` | Inteiros decimais | `inteiro idade bota 18` |
| `tronxo` | Reais com vírgula decimal | `tronxo media bota 7,5` |
| `papo` | `evera` ou `pala` | `papo ativo bota evera` |
| `resenha` | Texto entre `vei` e `tendeu` | `resenha nome bota vei Pedro tendeu` |

Há tipagem estática, sem inferência. Declarações têm a forma
`tipo identificador bota expressao`, sempre com inicialização. Atualizações usam
`identificador bota expressao`, preservando o tipo. O nome deve estar declarado e
visível; o inicializador é analisado antes da disponibilização do novo nome.
Não há sombreamento: não se pode redeclarar um nome já visível, nem em bloco
interno. Blocos independentes podem reutilizar nomes que deixaram de ser visíveis.

Operações numéricas mistas e atribuições para `tronxo` admitem promoção de
`inteiro`. Não há conversão implícita inversa nem entre números, textos e booleanos.
Limites numéricos concretos ainda serão documentados na implementação.

## Expressões

Os operadores aritméticos são `+`, `-`, `*`, `/`, `//`, `%`, `**` e `abs(...)`.
`/` é divisão real mesmo entre inteiros; `//` e `%` exigem inteiros; `**` produz
`tronxo`; `abs` preserva o tipo numérico. `+` não concatena textos. Parênteses
agrupam expressões. Comparações usam `<`, `<=`, `>`, `>=`, `==`, `!=`, sem
encadeamento; lógica usa `ai_dento`, `e`, `ou`, com operandos `papo`.

A precedência decrescente é: parênteses/chamadas; potência; sinais unários;
produto/divisões/resto; soma/subtração; comparações; negação lógica; conjunção;
disjunção. Consulte [gramática](grammar.md) e [semântica](semantics.md) para
associatividade, restrições e pontos ainda abertos sobre comparações.

## Programa, blocos e fluxo

Um programa tem pelo menos uma instrução no nível superior, executada em ordem,
sem `main` ou delimitador global. Apenas comentários ou linhas vazias não bastam.
Cada instrução comum ocupa uma linha; não há continuação de expressão. LF e CRLF
são aceitos, linhas vazias também, e EOF termina a última instrução.

`bora` abre um bloco e `arremata` fecha o mais próximo ainda aberto. Após ambos
há término de linha; blocos podem conter outras estruturas de controle.

```text
inteiro idade bota 18
vai_que (idade >= 18) bora
    desembucha(vei Maior de idade tendeu)
arremata
se_num_for (idade >= 16) bora
    desembucha(vei Entre 16 e 17 tendeu)
arremata
pois_pronto bora
    desembucha(vei Menor de 16 tendeu)
arremata
```

Cada ramo possui bloco próprio. `se_num_for` exige condição e completa um
`vai_que`; `pois_pronto` é o ramo restante, sem condição. Condições exigem `papo`.

`num_para ate (condicao) bora` testa uma **condição de parada** antes de executar
o corpo: se ela for `evera`, encerra; o corpo pode executar zero vezes.
`arrodeia` também prevê inicialização, condição de parada com `ate` e atualização,
separadas por `;`. Sua gramática detalhada permanece pendente.
`canso` interrompe e `simbora` pula uma repetição; só são aceitos dentro de laços.

## Entrada, saída, textos e comentários

`diga_ai(variavel)` recebe um identificador já declarado e armazena entrada
compatível com seu tipo. `desembucha(argumentos)` exibe valores, separados no
código por `;`, pois a vírgula integra literais reais. Inteiros são exibidos sem
decimais e `tronxo` com vírgula e três casas decimais por padrão. Detalhes de
arredondamento, separadores de saída e formatos de entrada precisam de confirmação.

Textos começam por `vei` e terminam por `tendeu`, com espaço ou tabulação
separando os delimitadores do conteúdo, sem aspas. O código usa ASCII.
`primeiramente` comenta até a quebra de linha, inclusive após instrução.
Comentários multilinha usam `espia` e `arriba`, entre instruções completas,
sem aninhamento. Marcadores de comentário dentro de textos são conteúdo literal.
Veja [tokens](tokens.md) para lacunas relativas a textos e espaços.

## Diagnósticos previstos

Erros léxicos, sintáticos e semânticos impedem a tradução; a análise encerra no
primeiro erro. Durante a execução poderão ocorrer divisão por zero, entrada
inválida/ausente, erros de domínio da potência e resultados numéricos fora das
faixas válidas. As verificações são futuras; detalhes em [semântica](semantics.md).
Os [exemplos](../examples/README.md) incluem cálculo, decisão, laço e casos inválidos.

## Extensões futuras

Funções/procedimentos só serão tratados após o núcleo funcional. As grafias
canônicas são `aperreio`, `toma`, `sem_futuro`. A proposta prevê parâmetros
tipados separados por `;`, promoção de argumentos inteiros para `tronxo`, retorno
compatível e declarações de função apenas no nível superior. Não há função
principal. A sintaxe completa não está fixada; não se oferece exemplo executável.

`liso` e `balaio` constam somente no quadro de equivalências. Não são tipos ou
valores especificados para uso. Vetores e indexação estão expressamente fora
desta versão; a menção de lista não altera esse escopo.

## Pontos a confirmar

| Ocorrência na fonte | Tratamento e impacto |
| --- | --- |
| §2.6 usa `funcao`; tabelas dos §§3 e 8 usam `aperreio` | Adotar `aperreio` para a extensão; confirmar a revisão da descrição e a sintaxe de funções |
| §3 menciona campos de `Arrodeia` | Usar somente `arrodeia`, conforme tabelas; definir cabeçalho detalhado antes do parser |
| §§2.3, 2.4, 4 e 5 ainda empregam `int`/`bool` ou terminologia usual | Tipos-fonte são `inteiro`, `tronxo`, `papo`, `resenha`; não aceitar aliases antigos |
| §3 diz que espaços são significativos | Não deduzir indentação sintática; confirmar tratamento fora de textos e limites de palavras |
| §8 inclui `liso` e `balaio` sem regras | Manter pendentes e fora dos exemplos do núcleo |
| §6, caso 2, contém `vai_que 2x)` e tabulações verticais; explicação alega falta de `)` antes de `bora`, que nem aparece | Não reutilizar como caso isolado; exemplos derivados devem ter falha inequívoca |
| OX-01 atribui à especificação original divisão inteira por truncamento para zero; §2.3 da cópia entregue só exemplifica operandos positivos | Confirmar a regra para negativos e o resto associado; não adotar automaticamente o piso de Python |
| §7 contém instruções separadas por tabulações verticais | Corrigir apenas os exemplos derivados para LF, mantendo a fonte intacta |

Também precisam de confirmação a formatação de reais com três casas
(arredondamento), os formatos de entrada e separadores de saída, as faixas e
a precisão numéricas e as regras completas da extensão de funções. Os
[riscos do destino Python](architecture.md#riscos-da-tradução-para-python-3)
orientam os cards futuros sem definir soluções nesta tarefa.

Outras lacunas específicas: [léxico](tokens.md#pontos-a-confirmar),
[sintaxe](grammar.md#pontos-a-confirmar) e [semântica](semantics.md#pontos-a-confirmar).
A [arquitetura](architecture.md) distingue o planejamento do estado implementado.
