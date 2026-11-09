# Runtime Python 3

O programa emitido pela CLI incorpora `src/runtime/oxente_runtime.py` e pode
ser executado fora do repositório, sem módulos auxiliares.

## Diagnósticos numéricos implementados

Todos os erros abaixo escrevem em stderr com etapa `execução`, linha da origem
Oxente e código de saída 1, sem traceback. A avaliação para na primeira falha.

| Situação | Código |
| --- | --- |
| Entrada real `nan`, `inf`, `-inf` ou texto numérico convertido em não finito | `entrada_invalida` |
| Resultado real não finito em `+`, `-`, `*` ou `/` | `real_fora_dos_limites` |
| Conversão de inteiro fora da representação real, explícita na emissão ou implícita em aritmética mista | `real_fora_dos_limites` |
| Divisor zero em `/`, `//` ou `%` | `dividiu_por_nada` |
| Domínio inválido, conversão impossível ou resultado não finito em `**` | `potencia_invalida` |

O código `real_fora_dos_limites` identifica a proteção de representação desta
implementação; não define uma faixa numérica nova para a linguagem.
A promoção inteiro → tronxo em declarações e atribuições passa por `_ox_tronxo`.
A divisão real também protege a conversão de ambos os operandos. O divisor zero
é verificado antes dessas conversões. Soma, subtração e multiplicação com tipo
tronxo usam helpers com captura restrita a `OverflowError`, incluindo a conversão
implícita de inteiros feita pelo Python. O resultado é verificado imediatamente,
mesmo sem `desembucha` posterior. Não há captura global de exceções do programa.

Os argumentos são avaliados da esquerda para a direita, uma vez cada. `e` e `ou`
continuam usando curto-circuito do Python; seus operandos não são antecipados.
Operações só de inteiros permanecem inteiras. `//` e `%` não passam por `float`,
preservando precisão acima de `2**53`. A potência mantém seu diagnóstico próprio.
Entradas reais finitas com vírgula ou ponto continuam aceitas, e a saída real
mantém três casas decimais com vírgula.

## Pendências separadas

A faixa inteira, a gramática exata de stdin (inclusive `1_000`), os detalhes de
arredondamento, a divisão/resto negativos e as regras provisórias de fluxo ainda
precisam das tarefas e decisões correspondentes. Python aceita inteiros grandes;
isso não aprova inteiros ilimitados como regra da linguagem. A proteção acima
trata a falha de representação real sem decidir a faixa inteira.

A validação desta correção é local em Linux. Windows e WSL continuam pendentes,
assim como a CI do próximo head publicado.
