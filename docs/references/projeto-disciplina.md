# **Projeto de Compiladores (60% da Nota)**

### **1\. Objetivo**

O projeto tem como objetivo aplicar os conceitos estudados na disciplina de Compiladores por meio do desenvolvimento de um tradutor ou interpretador para uma linguagem de programação criada pelo próprio grupo.

A implementação deverá contemplar análise léxica, análise sintática, construção de uma árvore sintática abstrata, análise semântica e tradução ou execução de programas.

### **2\. Formação das equipes**

O projeto poderá ser desenvolvido em dupla ou em trio.

Cada equipe deverá escolher uma das seguintes modalidades:

* **Tradutor:** recebe um programa escrito na linguagem criada pelo grupo e o converte para outra linguagem ou representação que permita produzir um resultado executável.  
* **Interpretador:** recebe um programa escrito na linguagem criada pelo grupo, analisa sua estrutura e executa suas instruções.

A linguagem de origem deverá ser criada pelo próprio grupo. A proposta deverá ser aprovada pelo professor antes do início da implementação.

### **3\. Tecnologias e ferramentas**

A implementação deverá respeitar as seguintes regras:

* A linguagem de implementação deverá ser C/C++.  
* Flex e Bison deverão ser utilizados para implementar os analisadores léxico e sintático.  
* Os arquivos de especificação utilizados pelo Flex e pelo Bison deverão ser entregues.  
* O uso de Git será obrigatório. O grupo deverá disponibilizar o repositório e manter o histórico de desenvolvimento.  
* O projeto deverá incluir instruções para compilação e execução.

### **4\. Criação e especificação da linguagem**

Cada grupo deverá criar e documentar uma linguagem própria, definindo:

1. Nome e finalidade da linguagem.  
2. Conjunto de tokens e respectivas regras léxicas.  
3. Gramática formal.  
4. Precedência e associatividade dos operadores, quando aplicáveis.  
5. Regras de declaração e utilização de variáveis.  
6. Regras semânticas.  
7. Exemplos de programas válidos e inválidos.

Os grupos terão liberdade para definir a sintaxe e as regras da linguagem, desde que respeitem os requisitos mínimos deste documento.

As decisões de projeto deverão ser documentadas na proposta inicial e submetidas à aprovação do professor.

### **5\. Funcionalidades obrigatórias**

Todos os projetos deverão implementar, no mínimo, as funcionalidades descritas a seguir.

**5.1. Funcionalidades da linguagem**

* Expressões com operações aritméticas e regras definidas de precedência e associatividade.  
* Variáveis e armazenamento de valores.  
* Comandos de atribuição.  
* Um mecanismo para exibição de resultados.  
* Pelo menos uma estrutura condicional, como if.  
* Pelo menos uma estrutura de repetição, como while.

A sintaxe específica desses comandos será definida pelo grupo e documentada na gramática da linguagem.

**5.2. Análise léxica**

O programa deverá reconhecer os tokens definidos na linguagem e identificar entradas que contenham erros léxicos, apresentando mensagens que auxiliem na identificação do problema.

**5.3. Análise sintática**

O programa deverá verificar se o código-fonte respeita a gramática definida pelo grupo e identificar entradas sintaticamente inválidas.

**5.4. Árvore sintática abstrata (AST)**

Todos os grupos deverão construir uma árvore sintática abstrata explícita para representar a estrutura dos programas reconhecidos.

A AST deverá ser utilizada nas etapas posteriores do processamento. Não será suficiente construí-la e descartá-la sem utilizá-la na tradução ou interpretação.

**5.5. Tabela de símbolos**

O projeto deverá manter uma estrutura para registrar e consultar identificadores da linguagem, como variáveis, de acordo com as regras definidas pelo grupo.

**5.6. Análise semântica**

Todos os grupos deverão implementar verificações semânticas mínimas, incluindo:

* Verificação do uso de variáveis conforme as regras de declaração definidas pela linguagem.  
* Verificação da utilização válida de identificadores.  
* Verificação da compatibilidade das operações com os valores envolvidos, quando aplicável.  
* Verificação de outras restrições necessárias para identificar programas semanticamente inválidos.

As regras semânticas adotadas deverão ser documentadas, e os testes deverão demonstrar o funcionamento das verificações implementadas.

**5.7. Tratamento de erros**

O projeto deverá identificar e informar erros léxicos e sintáticos. Também deverá realizar as verificações semânticas obrigatórias e apresentar mensagens adequadas quando identificar violações das regras da linguagem.

**5.8. Tradução ou interpretação**

O projeto deverá implementar integralmente a modalidade escolhida, conforme os requisitos da Seção 6\.

### **6\. Requisitos específicos por modalidade**

**6.1. Tradutor**

O tradutor deverá converter os programas da linguagem criada para um destino definido pelo grupo.

O destino poderá ser outra linguagem de programação. A escolha deverá ser justificada na proposta inicial e aprovada pelo professor.

O grupo deverá demonstrar que o resultado gerado pode ser executado e que seu comportamento corresponde ao esperado para os programas suportados pela linguagem.

**6.2. Interpretador**

O interpretador deverá executar os programas da linguagem criada, respeitando as regras sintáticas e semânticas definidas pelo grupo.

A execução deverá utilizar a AST construída durante o processamento, produzindo os resultados esperados e identificando situações inválidas conforme as regras da linguagem.

### **7\. Complexidade e funcionalidades adicionais**

Todos os grupos deverão implementar, no mínimo, o conjunto obrigatório de funcionalidades. Além dele, poderão propor extensões, como:

* Tipos de dados adicionais.  
* Operadores relacionais e lógicos.  
* Funções, parâmetros e valores de retorno.  
* Regras de escopo.  
* Estruturas de dados adicionais.  
* Otimizações ou geração de código intermediário.

As funcionalidades adicionais deverão ser descritas na proposta inicial. 

A implementação de funcionalidades extras não substituirá os requisitos obrigatórios.

### **8\. Uso de inteligência artificial**

O uso de ferramentas de inteligência artificial será permitido, e incentivado, durante o desenvolvimento, observando as seguintes condições:

* O grupo deverá declarar as ferramentas utilizadas e as atividades em que foram empregadas.  
* Os integrantes serão responsáveis pela correção e pelo funcionamento do código entregue.  
* Todos deverão compreender as decisões de implementação e ser capazes de explicar as etapas do projeto.  
* O uso de ferramentas de IA não dispensará o domínio técnico necessário para a apresentação e demonstração do projeto.

### **9\. Entregas e cronograma**

O projeto será desenvolvido em três etapas.

**Etapa 1 — Proposta e especificação inicial**

**Data: 22/10/2026**

O grupo deverá apresentar:

* Integrantes da equipe.  
* Nome e finalidade da linguagem.  
* Modalidade escolhida: tradutor ou interpretador.  
* Exemplos de programas na linguagem proposta.  
* Funcionalidades planejadas.

A proposta deverá ser aprovada pelo professor antes do avanço para a implementação completa.

**Etapa 2 — Versão intermediária funcional**

**Data: 12/11/2026**

O grupo deverá apresentar uma versão funcional que demonstre:

* Gramática inicial e decisões de sintaxe e semântica.  
* Análise léxica.  
* Análise sintática.  
* Construção da AST.  
* Progresso da implementação da modalidade escolhida.  
* Exemplos de execução ou processamento.  
* Testes iniciais, incluindo casos válidos e inválidos.

O professor poderá solicitar ajustes no escopo ou na implementação para garantir o cumprimento dos requisitos obrigatórios.

**Etapa 3 — Entrega final e apresentação**

**Data: 26/11/2026**

O grupo deverá entregar o projeto completo, incluindo:

* Código-fonte.  
* Arquivos de especificação do Flex e do Bison.  
* Especificação da linguagem e gramática formal.  
* Documentação técnica da arquitetura e das decisões de implementação.  
* Testes e resultados esperados.  
* Instruções de compilação e execução.  
* Repositório Git com histórico de desenvolvimento.

Na mesma etapa, o grupo deverá apresentar e demonstrar o funcionamento do projeto.

### **10\. Apresentação**

A apresentação será realizada coletivamente, com participação de todos os integrantes.

O grupo deverá explicar a linguagem criada, a arquitetura da implementação, as principais decisões de projeto e o funcionamento das etapas de processamento.

O professor poderá fazer perguntas sobre qualquer parte do projeto e solicitar a execução de novos programas para verificar seu funcionamento.

Todos os integrantes deverão demonstrar domínio técnico do trabalho desenvolvido.

### **11\. Critérios de avaliação**

A nota será individual, com uma única nota para a equipe, totalizando 100 pontos percentuais.

| Critério | Peso |
| ----- | ----- |
| Análise léxica e sintática | 20% |
| AST e tabela de símbolos | 15% |
| Análise semântica e tratamento de erros | 15% |
| Tradução ou interpretação | 20% |
| Testes e corretude | 10% |
| Documentação e reprodutibilidade | 10% |
| Apresentação e demonstração | 10% |
| **Total** | **100%** |

### **12\. Disposições finais**

O cumprimento dos requisitos obrigatórios será necessário para a avaliação integral do projeto.

As decisões de implementação que não estiverem explicitamente definidas neste documento deverão ser descritas na proposta e submetidas à aprovação do professor.

O professor poderá solicitar esclarecimentos, correções ou demonstrações adicionais para verificar o cumprimento dos requisitos e a compreensão do trabalho desenvolvido.

