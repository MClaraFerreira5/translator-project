# Suíte de Testes Automatizada da Oxente Lang

Este documento descreve a organização, cobertura, estratégia de execução automatizada e as limitações conhecidas da suíte de testes da linguagem Oxente Lang (Card 15).

A suíte foi concebida para transformar exemplos e casos inválidos da especificação em verificações automatizadas de forma progressiva: ela valida rigorosamente os módulos já entregues (como o scanner léxico e helpers de runtime) e acompanha o avanço dos cards em desenvolvimento (parser, analisador semântico e gerador de código) **sem mascarar testes falhos nem emitir falsos sucessos**.

---

## 1. Estrutura e Catálogo de Testes

A suíte é orientada por um catálogo estruturado em formato legível por máquina (`tests/suite_manifest.json`), no qual cada cenário define:
- **Identificador**: código único do caso (ex.: `val-calculo`, `inval-semantico-tipo`).
- **Arquivo de entrada**: programa `.oxe` correspondente em `examples/valid/` ou `examples/invalid/`.
- **Tipo**: `valido` ou `invalido`.
- **Etapa responsável**: `lexica`, `sintatica`, `semantica` ou `runtime`.
- **Tópicos cobertos**: categorias da especificação exercitadas.
- **Resultado esperado**: comportamento previsto ou código/diagnóstico esperado.
- **Status de implementação**: `pronto` (para módulos já implementados) ou `pendente_card` (para módulos em construção).

---

## 2. Matriz de Cobertura dos Critérios de Aceite

A suíte cobre 100% dos tópicos exigidos pela especificação:

| Critério de Aceite | Casos que exercitam | Descrição do teste |
| :--- | :--- | :--- |
| **Precedência** | `calculo.oxe`<br>`precedencia-potencia.oxe`<br>`precedencia-logica.oxe` | Multiplicação antes de soma; `**` associativo à direita com maior prioridade que unário; `ai_dento` > `e` > `ou`; relacionais antes de lógicos. |
| **Declaração antes do uso** | `semantico-nao-declarado.oxe`<br>`semantico-uso-proprio-inicializador.oxe` | Uso de identificador ausente; uso de variável na expressão do próprio inicializador (`inteiro x bota x + 1`). |
| **Tipos incompatíveis** | `semantico-tipo.oxe`<br>`semantico-operacao-tipo.oxe` | Atribuição de `tronxo` para `inteiro` sem conversão implícita; operações aritméticas inválidas (`inteiro` + `resenha`). |
| **Duplicação** | `semantico-duplicacao.oxe` | Redeclaração de variável com mesmo nome no mesmo escopo. |
| **Escopos** | `semantico-sombreamento.oxe`<br>`semantico-escopo-fechado.oxe`<br>`escopo-independente.oxe` | Proibição de sombreamento em blocos aninhados; acesso a variáveis após o fechamento do bloco com `arremata`; reutilização válida em blocos irmãos. |
| **Condições não booleanas** | `semantico-condicao-nao-booleana.oxe`<br>`semantico-condicao-repeticao-nao-booleana.oxe` | Condição de `vai_que` usando inteiro em vez de `papo`; condição de `num_para ate` usando número. |
| **Operadores longos** | `operadores-longos.oxe` | Operadores multi-caractere: `>=`, `<=`, `==`, `!=`, `//`, `**`, `ai_dento`, `se_num_for`, `pois_pronto`. |
| **Textos e comentários** | `comentarios.oxe`<br>`lexico-texto-nao-terminado.oxe`<br>`lexico-comentario-nao-terminado.oxe` | Delimitadores `vei ... tendeu`, comentários de linha `primeiramente`, bloco `espia ... arriba`, e rejeição de literais/comentários não terminados. |
| **NL (quebras de linha)** | `nl-crlf-eof.oxe` | Quebras de linha consecutivas, comentários intercalados e término no EOF sem quebra obrigatória final. |
| **Construções aninhadas** | `aninhamento.oxe` | Condicionais com múltiplos ramos aninhados dentro de laço de repetição `num_para ate`. |
| **Erros de gramática** | `sintatico-operando.oxe`<br>`sintatico-parentese-desbalanceado.oxe`<br>`sintatico-bloco-nao-fechado.oxe`<br>`sintatico-comparacao-encadeada.oxe`<br>`sintatico-tipo-sem-identificador.oxe` | Falta de operando; parênteses não balanceados; bloco sem `arremata`; encadeamento `1 < 2 < 3`; declaração sem identificador. |

---

## 3. Como Executar a Suíte

### Execução via Python (Runner direto)
Com os binários compilados em `build/`, execute:
```powershell
python tests/executar_suite.py
```

### Execução via CMake / CTest
A suíte também pode ser executada por meio do runner de testes do CMake:
```powershell
ctest --test-dir build --output-on-failure
```

---

## 4. Resultados Reais e Limitações Conhecidas

Em conformidade com as diretrizes do projeto, os testes reportam o estado real dos componentes sem mascarar funcionalidades incompletas:

1. **Etapa Léxica (Aprovada - 100%)**:
   - O scanner Flex (`oxente-tokens`) valida com precisão tanto os casos válidos (palavras canônicas, literais, comentários, quebras) quanto a detecção de caracteres inválidos e delimitações incompletas.
2. **Etapa Sintática e Semântica (Limitação Conhecida - Em desenvolvimento)**:
   - O compilador CLI (`oxente --check`) atualmente retorna código `4` (`não implementado`) para a árvore AST e tabela de símbolos em integração.
   - A suíte identifica essas execuções como `[LIMITAÇÃO CONHECIDA]`, documentando que os testes estão prontos e aguardando a finalização dos cards de parser e análise semântica.
3. **Runtime Python (Aprovado)**:
   - Os helpers de execução e comportamento de operadores/erros de execução são verificados independentemente via `tests/test_runtime.py` (teste `runtime` no CTest).
4. **Gerador Python (Aprovado, sem parser)**:
   - `tests/test_codegen.cpp` (teste `codegen` no CTest) monta ASTs manualmente, gera o Python, executa-o com Python 3 e compara a saída. Cobre expressões, tipos, nomes internos, condicionais, laços aninhados, `canso`, `simbora` e escopos.
   - Os arquivos `.oxe` do catálogo ainda não passam pelo gerador, pois dependem do parser.
