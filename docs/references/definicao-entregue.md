# **Definição da linguagem de programação**

## **1\. Identidade: cada grupo deverá definir o nome da linguagem**

Oxente Lang

**2\. Funcionalidades da linguagem**

### **2.1. Tipos de dados**

Quais tipos de valores a linguagem permitirá?

| Tipo | Valores permitidos | Exemplo de declaração |
| :---- | :---- | :---- |
| inteiro | Números inteiros | inteiro idade bota 18 |
| tronxo | Números reais com vírgula decimal | tronxo media bota 7,5 |
| papo | evera ou pala | papo ativo bota evera |
| resenha | Texto delimitado por “vei “ e “ tendeu” | resenha nome bota vei Pedro tendeu |

Questões para discutir: a linguagem terá tipos de dados? Será possível misturar números inteiros e reais? Como os textos serão representados?

Sim. A linguagem terá tipagem estática: os tipos serão declarados e verificados antes da geração do código. Expressões numéricas poderão combinar inteiro e tronxo; nesse caso, o inteiro será promovido a tronxo. A promoção também será permitida ao atribuir um inteiro a uma variável tronxo. A conversão implícita de tronxo para inteiro será rejeitada, assim como conversões entre números, textos e booleanos. Os textos serão delimitados por vei e tendeu, sem aspas.

### **2.2. Variáveis e declarações**

* Como criar uma variável?

A declaração terá a forma tipo identificador bota expressao. Toda declaração deverá inicializar a variável com um valor compatível com o tipo informado.

Exemplos:

```
inteiro pontos bota 0
```

```
tronxo media bota 7,5
```

```
resenha nome bota vei Pedro tendeu
```

* Será necessário declarar a variável antes de utilizá-la?

Sim. O identificador deverá estar declarado e visível antes de ser utilizado. O analisador semântico consultará a tabela de símbolos para verificar essa condição.

* A declaração deverá informar o tipo?

Sim. Os tipos disponíveis serão inteiro, tronxo, papo e resenha. Não haverá inferência de tipos na versão inicial.

* Será permitido alterar o valor de uma variável?

Sim. A atualização utilizará identificador bota expressao, sem repetir o tipo. O tipo da variável permanecerá fixo durante seu tempo de vida.

```
pontos bota pontos + 5
```

* Será possível criar uma variável com o mesmo nome de outra?

Não será permitida uma nova declaração com um nome já visível, inclusive em um bloco interno. Portanto, não haverá sombreamento. Nomes iguais poderão ser usados em blocos independentes, quando a declaração anterior já não estiver visível. Uma atribuição ao identificador existente não constitui uma nova declaração.

* Como a linguagem tratará o uso de uma variável que não foi declarada?

O uso será rejeitado na análise semântica, antes da tradução. A mensagem identificará a variável, a localização e a ausência de declaração. Exemplo: “Tais bestando? Variável quantidade não declarada.”

O grupo deverá escolher uma abordagem e justificar a decisão.

Justificativa: a declaração explícita e a inicialização obrigatória facilitam a leitura do programa, a construção da tabela de símbolos e a identificação de erros de uso e de tipos, sem exigir análise de inicialização em diferentes caminhos de execução.

### **2.3. Operadores e expressões**

* Quais operações aritméticas existirão?

| Operação | Notação | Regra e exemplo |
| :---- | :---- | :---- |
| Soma e subtração | \+ e \- | 2 \+ 3 \= 5; 5 \- 2 \= 3 |
| Multiplicação | \* | 3 \* 4 \= 12 |
| Divisão real | / | 5 / 2 \= 2,5, mesmo com operandos int |
| Divisão inteira | // | Somente inteiro; 7 // 2 \= 3 |
| Resto | % | Somente inteiro; 7 % 2 \= 1 |
| Potenciação | \*\* | 2 \*\* 3 \= 8; resultado do tipo tronxo |
| Valor absoluto | abs(expr) | abs(-5) \= 5; preserva o tipo numérico |

* Haverá operadores relacionais, como \>, \< e \==?\\  
  * Sim: \>, \<, \>=, \<=, \== e \!=.

* Haverá operadores lógicos, como and, or e not?  
  * Sim, os operadores serão e, ou e ai\_dento  
* Qual será a precedência dos operadores?

| Prioridade | Operadores ou construção | Associatividade |
| :---- | :---- | :---- |
| 1 • maior | Parênteses e chamadas | Agrupamento explícito |
| 2 | \*\* | Direita para esquerda |
| 3 | \+ e \- unários | Direita para esquerda |
| 4 | \*, /, //, % | Esquerda para direita |
| 5 | \+ e \- binários | Esquerda para direita |
| 6 | \<, \<=, \>, \>=, \==, \!= | Sem encadeamento |
| 7 | ai\_dento | Direita para esquerda |
| 8 | e | Esquerda para direita |
| 9 • menor | ou | Esquerda para direita |

* Como será definida a associatividade?  
  * A subtração 10 \- 3 \- 2 será interpretada como (10 \- 3\) \- 2, resultando em 5\. A potenciação 2 \*\* 3 \*\* 2 será interpretada como 2 \*\* (3 \*\* 2), resultando em 512,0. Como a potenciação precede o sinal unário, \-2 \*\* 2 equivale a \-(2 \*\* 2), resultando em \-4,0

* Será permitido usar parênteses para agrupar expressões?  
  * Sim;

Desafio: definir como a linguagem interpretará 2 \+ 3 \* 4 e (2 \+ 3\) \* 4\. O grupo deve explicar por que os resultados são diferentes.

### **2.4. Comandos de controle de fluxo**

* Como serão escritos os comandos condicionais?  
  * As condições serão escritas com vai\_que(se), seguido de uma expressão bool entre parênteses e de um bloco. se\_num\_for(elif) introduzirá uma condição alternativa, e pois\_pronto(else) representará o caso restante.

* Haverá else e condicionais encadeadas?  
  * Sim. 

* Quais estruturas de repetição serão permitidas?  
  * Serão permitidos num\_para(while), para repetição controlada por uma condição de parada, e arrodeia(for), para repetição com inicialização, condição de parada e atualização. Em ambas as estruturas, ate introduzirá uma condição booleana de parada.

* Como será definida a condição de parada de um laço?  
  * ate (condicao) \\n  
* Existirão comandos para interromper ou pular uma repetição?  
  * Break: canso  
  * Continue: simbora  
* Como será representado um bloco de comandos?  
  * bora *expressao* arremata

Exemplos de decisões: usar chaves {}, indentação ou palavras como begin e end para delimitar blocos, ou outra coisa?

### **2.5. Entrada e saída de dados**

* Como o programa exibirá resultados?  
  * O programa exibirá resultados com desembucha(argumentos).

* Será possível receber dados do usuário? tipo input  
  * Sim. diga\_ai(variavel) receberá um identificador já declarado e armazenará nele uma entrada compatível com seu tipo.

* Como números e textos serão apresentados?  
  * Inteiros serão exibidos sem parte decimal;

  * Reais serão exibidos com o separador representado por vírgulas e com 3 casa decimais por padrão. Ex: 5,906

* Será possível exibir vários valores na mesma instrução?  
  * Sim. O separador será ponto e vírgula, porque a vírgula integra os números reais.

Por exemplo, o grupo pode propor comandos como print, show, read ou criar palavras reservadas próprias.

### **2.6. Funções e modularização**

* A linguagem terá funções?  
  * Sim, como extensão planejada após a conclusão do núcleo funcional. A palavra de declaração será funcao;  
* Será possível passar parâmetros?  
  * Sim, os parâmetros terão tipo e nome explícitos e serão separados por ponto e vírgula. Um argumento inteiro poderá atender a um parâmetro tronxo, mas a conversão inversa será rejeitada.  
* Uma função poderá retornar um valor?  
  * Sim. toma(return) expressao retornará um valor compatível com o tipo da função.

* Existirá uma função principal, como main?  
  * Não. Os comandos do nível superior serão executados em ordem;  
* O programa poderá ser dividido em procedimentos reutilizáveis?  
  * Sim;

Esse tópico pode ser tratado como uma funcionalidade opcional ou avançada, sem obrigar todos os grupos a implementá-la.

## **3\. Definição das regras léxicas**

Esta parte é muito importante para a disciplina, porque prepara diretamente a construção do analisador léxico com Flex.

Peça aos grupos que definam:

* Quais caracteres poderão aparecer nos programas?  
  * O código-fonte utilizará ASCII: letras, algarismos, sublinhado, espaços, tabulações, quebras de linha e os sinais necessários aos operadores e delimitadores;

* Como serão escritos os identificadores?  
  * A expressão regular será \[A-Za-z\_\]\[A-Za-z0-9\_\]\*.

* Um identificador poderá começar com número?  
  * Não;  
* Letras maiúsculas e minúsculas serão diferenciadas?  
  * Sim;  
* Quais serão as palavras reservadas?

| Categoria | Palavras reservadas |
| :---- | :---- |
| Tipos | inteiro, tronxo, papo, resenha |
| Declaração e blocos | bota, bora, arremata |
| Condicionais | vai\_que, se\_num\_for, pois\_pronto |
| Repetição | num\_para, arrodeia, ate, canso, simbora |
| Valores e lógica | evera, pala, ai\_dento, e, ou |
| Entrada, saída e cálculo | diga\_ai, desembucha, abs |
| Textos e comentários | vei, tendeu, primeiramente, espia, arriba |
| Extensão de funções | aperreio, toma, sem\_futuro |

* Como serão representados números inteiros e reais?  
  * Inteiros seguirão \[0-9\]+, como 0, 18 e 87\. Reais seguirão \[0-9\]+,\[0-9\]+, como 3,000.

* Como serão representados textos?  
  * Textos começarão com a palavra vei e terminarão com a palavra tendeu, separadas do conteúdo por espaço ou tabulação.

* Quais operadores e símbolos especiais existirão?  
  * Os operadores simbólicos serão \+, \-, \*, /, //, %, \*\*, \>, \<, \>=, \<=, \== e \!=. Os parênteses agruparão expressões e delimitarão chamadas e cabeçalhos. O ponto e vírgula separará argumentos, parâmetros e os campos de Arrodeia; a vírgula pertencerá aos literais reais. A quebra de linha será um token de término de instrução.

* Espaços em branco e quebras de linha serão ignorados?  
  *  As quebras de linha e espaços em branco serão significativas, incluindo a que termina um comentário de linha. Linhas vazias serão aceitas. Serão reconhecidos finais de linha LF e CRLF; a ausência de quebra de linha antes do fim do arquivo não invalidará a última instrução.

* A linguagem permitirá comentários? Como serão escritos?  
  * Sim. primeiramente iniciará um comentário até a quebra de linha, podendo aparecer após uma instrução. Comentários de várias linhas começarão por espia e terminarão por arriba, escritos entre instruções completas e sem aninhamento. As palavras que marcam comentários não serão interpretadas dentro de um literal de texto. A contagem de linhas será mantida durante a leitura dos comentários.

Por exemplo, cada grupo deverá decidir se idade, Idade e IDADE representam o mesmo identificador ou três identificadores diferentes.

Também deverá definir exemplos de identificadores válidos e inválidos.

## **4\. Definição das regras sintáticas**

O objetivo é começar a pensar na estrutura dos comandos, sem exigir ainda que os alunos escrevam toda a gramática formal.

Peça que respondam:

* Como termina uma instrução: com ;, quebra de linha ou outro símbolo?

Uma instrução comum terminará com uma quebra de linha, e não com ponto e vírgula. Será adotada uma instrução por linha, sem continuação de expressões entre linhas. O fim do arquivo equivalerá ao término da última linha. Linhas vazias poderão aparecer entre instruções.

* Como será escrita uma atribuição?

Uma atribuição terá a forma identificador bota expressao. A declaração acrescentará o tipo antes do identificador: int total bota 10\. Uma atualização será escrita como total bota total \+ 2\. \== representará comparação, sem atribuição.

* Como será representada uma expressão matemática?

Uma expressão será composta de literais, identificadores, operadores, parênteses e abs(...); chamadas de funções com retorno serão admitidas na extensão. A precedência e a associatividade seguirão a seção 2.3. Exemplo: tronxo media bota (7 \+ 8\) / 2\.

* Como será escrita uma estrutura condicional?

A forma básica será vai\_que (expressao) bora, seguida do corpo e de arremata. Poderão seguir ramos se\_num\_for e pois\_pronto. Cada ramo terá seu próprio bloco, e todos os cabeçalhos condicionais exigirão parênteses. O exemplo completo está na seção 2.4.

* Como será delimitado um bloco?

bora abrirá o bloco e arremata o encerrará. Após bora e arremata haverá término de linha. Cada arremata fechará o bora ainda aberto mais próximo, permitindo aninhamento explícito.

* Uma instrução poderá conter outra instrução?

Sim. Condicionais e laços conterão sequências de instruções e poderão conter outras estruturas de controle. Declarações de função serão restritas ao nível superior. Restrições de contexto, como canso fora de laço, serão verificadas semanticamente.

* Qual será a estrutura mínima de um programa completo?

Um programa executável mínimo conterá uma instrução válida no nível superior, sem main ou delimitador global obrigatório. Comentários e linhas vazias, isoladamente, não constituirão um programa executável para esta proposta.

```
desembucha(vei Ola mundo tendeu)
```

* Quais combinações de comandos serão permitidas?

Serão permitidas sequências de declarações, atribuições, entrada, saída, condicionais e laços. Dentro de laços, serão admitidos canso e simbora. Na extensão, definições de função aparecerão no nível superior e chamadas poderão compor expressões quando houver retorno, ou constituir uma instrução. se\_num\_for e pois\_pronto só poderão completar um vai\_que.

## **5\. Definição das regras semânticas**

Esta é uma oportunidade de mostrar que um programa pode estar sintaticamente correto, mas ainda assim não fazer sentido.

Peça que os alunos discutam:

* O que acontece quando uma variável não existe?

O programa será rejeitado antes da geração de código. A tabela de símbolos registrará tipo, escopo e localização da declaração. A mensagem indicará o nome não encontrado, a linha e a coluna quando disponível.

* Uma variável pode ser utilizada antes de receber um valor?

Não. Toda declaração exigirá um inicializador. O inicializador será analisado antes de tornar a nova variável disponível; portanto, int x bota x \+ 1 será inválido se x ainda não existir. Parâmetros de função serão inicializados pelos argumentos na chamada.

* É permitido somar um número a um texto?

Não. O operador \+ será numérico, sem concatenação de texto ou conversão implícita. Para apresentar um número junto de um texto, será utilizada uma saída com vários argumentos: desembucha(10; vei pontos tendeu).

* Uma condição de if deve obrigatoriamente produzir um valor booleano?

Sim. vai\_que e se\_num\_for exigirão papo. O mesmo valerá para a condição após ate nos laços. inteiro e resenha não serão convertidos em verdadeiro ou falso. evera e pala serão os únicos literais booleanos; por exemplo, papo ativo bota evera.

* O que acontece quando uma divisão por zero é executada?

O programa gerado será interrompido com dividiu\_por\_nada, identificando a operação e a linha da Oxente Lang. O tradutor gerará verificações para /, // e %.

* O que acontece quando uma variável recebe um valor incompatível com seu tipo?

Será emitido misturou\_as\_coisas, informando o tipo esperado e o encontrado. A atribuição de int para tronxo será aceita por promoção; tronxo para int, texto para número e número para papo serão rejeitados. A mesma compatibilidade será aplicada a operações, parâmetros e retornos, quando a extensão estiver habilitada.

* Como serão tratados nomes duplicados?

Uma declaração será rejeitada com valor\_sem\_pe\_nem\_cabeca quando seu nome já estiver visível. Cada bloco terá escopo próprio, com busca do escopo mais interno para o externo. Na extensão, funções ocuparão um espaço global de nomes, sem sobrecarga, e não poderão colidir com identificadores visíveis; os nomes de parâmetros não poderão se repetir na mesma função.

* Quais erros devem ser detectados antes da execução e quais podem ocorrer durante a execução?

Antes da geração serão detectados caracteres inválidos, textos ou comentários não terminados, estruturas fora da gramática, nomes não declarados ou duplicados, operações e atribuições incompatíveis, condições não booleanas e interrupções fora de laços. Na extensão, serão verificadas também chamadas, parâmetros e retornos. Durante a execução poderão ocorrer divisão por zero, entrada inválida ou ausente e erros de domínio da potenciação. O projeto não incluirá vetores ou indexação; logo, acesso a índice fora dos limites não fará parte desta versão.

O comportamento numérico será considerado dentro das faixas dos tipos escolhidos. Operações que excedam a faixa inteira ou produzam valores reais não finitos não constituirão resultados válidos. A implementação documentará os limites e incluirá verificações nas operações numéricas, evitando que o código gerado dependa de overflow inteiro com sinal.

## **6\. Erros e mensagens de diagnóstico**

Peça que cada grupo imagine como seu compilador ou interpretador se comportará diante de entradas incorretas.

Caso 1:

```
inteiro x bota 10 +
```

Caso 2:

```
inteiro x bota 12vai_que 2x)arremata
```

Caso 3:

```
inteiro y bota variavel_inexistente + 1
```

Para cada exemplo, os alunos deverão indicar:

* O programa é válido ou inválido?

Os três casos serão inválidos. No caso 1, falta o operando após \+. No caso 2, falta ) antes de bora. No caso 3, o identificador variavel\_inexistente não possui declaração anterior.

* Qual etapa identificaria o problema?

Casos 1 e 2: análise sintática. Os tokens podem ser reconhecidos, mas não formam uma construção válida. Caso 3: análise semântica, por falha na resolução do identificador.

* Que mensagem deveria ser apresentada?

Caso 1: Erro sintatico na linha \<n\>: esperada uma expressao apos \+.  
Caso 2: Erro sintatico na linha \<n\>: esperado ) antes de bora.  
Caso 3: Tais bestando? Variavel variavel\_inexistente nao declarada na linha \<n\>.

* O programa poderia continuar sendo analisado após o erro?

Não, a análise será encerrada no primeiro erro, com diagnóstico e status de falha. O usuário corrigirá o programa e executará novamente o tradutor. A coleta de múltiplos erros por recuperação sintática poderá ser acrescentada posteriormente, sem integrar os compromissos do núcleo.

## **7\. Exemplos de programas**

Cada grupo deverá escrever pelo menos três pequenos programas na linguagem que está projetando.

Programa 1: Cálculo

Declara valores, calcula duas expressões com agrupamentos diferentes e exibe seus resultados.

```
inteiro primeiro bota 2inteiro segundo bota 3inteiro terceiro bota 4inteiro resultado bota primeiro + segundo * terceirointeiro agrupado bota (primeiro + segundo) * terceirodesembucha(vei Resultado tendeu; resultado)desembucha(vei Com parenteses tendeu; agrupado)
```

Saída esperada:

```
Resultado 14Com parenteses 20
```

A multiplicação precede a soma do primeiro cálculo. No segundo, os parênteses fazem a soma ocorrer antes da multiplicação.

Programa 2: Decisão

Verifica uma condição composta, usando os três operadores lógicos definidos, e seleciona um ramo condicional.

```
inteiro idade bota 18papo autorizado bota everapapo bloqueado bota palavai_que ((idade >= 18 ou autorizado) e ai_dento bloqueado) bora    desembucha(vei Acesso permitido tendeu)arrematapois_pronto bora    desembucha(vei Acesso negado tendeu)arremata
```

Saída esperada: Acesso permitido. A condição exige que a pessoa seja maior de idade ou esteja autorizada e, em qualquer caso, não esteja bloqueada. Se bloqueado receber evera, a saída será Acesso negado.

Programa 3: Repetição

Acumula os inteiros de 1 a 5, encerrando a repetição quando o contador ultrapassa 5\.

```
inteiro contador bota 1inteiro soma bota 0num_para ate (contador > 5) bora    soma bota soma + contador    contador bota contador + 1arrematadesembucha(soma)
```

Saída esperada:

```
15
```

Quando o contador chega a 6, a condição de parada se torna evera e o corpo deixa de ser executado. Se a condição já fosse verdadeira no primeiro teste, o corpo executaria zero vezes.

## **8\. Tabelas dos nomes de Oxente Lang**

| Nome usual | Oxente Lang |
| ----- | ----- |
| `int` | `inteiro` |
| `float` | `tronxo` |
| `bool` | `papo` |
| `string` | `resenha` |
| `true` | `evera` |
| `false` | `pala` |
| Atribuição (`=`) | `bota` |
| `if` | `vai_que` |
| `else if` / `elif` | `se_num_for` |
| `else` | `pois_pronto` |
| `while` com condição de parada | `num_para ate` |
| `for` com condição de parada | `arrodeia (...; ate ...; ...)` |
| `break` | `canso` |
| `continue` | `simbora` |
| `not` | `ai_dento` |
| `and` | `e` |
| `or` | `ou` |
| Início de bloco (`{` / `begin`) | `bora` |
| Fim de bloco (`}` / `end`) | `arremata` |
| Null | `liso` |
| Lista | `balaio` |
| Função | `aperreio` |
| Return | `toma` |
| Void | `sem_futuro` |
| Saída (print) | `desembucha` |
| Entrada (input) | `diga_ai` |
| Início de texto (aspas de abertura) | `vei` |
| Fim de texto (aspas de fechamento) | `tendeu` |
| Comentário de uma linha | `primeiramente` |
| Início de comentário de várias linhas | `espia` |
| Fim de comentário de várias linhas | `arriba` |

