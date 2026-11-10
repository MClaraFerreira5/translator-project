# Contrato da linha de comando

```text
oxente --help
oxente MODO arquivo.oxe [-o caminho]
```

Exija exatamente um modo explícito e uma entrada regular, legível, com extensão
`.oxe` (sensível a maiúsculas). Não há modo padrão. A ordem dos argumentos é
livre; `--` termina as opções, permitindo entradas iniciadas por hífen.
`--help` deve ser usado sozinho. Opções desconhecidas, modos repetidos ou
incompatíveis, múltiplas entradas e argumentos ausentes são erros.

| Opção | Contrato e estado |
| --- | --- |
| `--check` | Valida o programa, sem gerar código |
| `--dump-ast` | Exibe a AST como texto indentado em stdout, sem análise semântica |
| `--emit-python` | Gera Python 3 em stdout ou no caminho de `-o` |
| `-o caminho`, `--output caminho` | Opcional, uma única vez e somente com `--emit-python`; não pode ser a própria entrada |

Caminhos de saída iniciados por hífen devem usar `./nome`. A sintaxe
`--output=caminho` não é suportada. A escrita em stdout quando não há `-o` é uma
escolha provisória.

| Código | Significado |
| --- | --- |
| 0 | Ajuda, validação, AST ou geração concluída |
| 1 | Erro léxico, sintático ou semântico no programa |
| 2 | Erro de argumentos/uso, incluindo extensão inválida |
| 3 | Entrada inexistente, não regular ou impossível de abrir; saída impossível de escrever |

Erros vão para stderr, sem saída normal em stdout. A CLI identifica a etapa e a localização nos diagnósticos. Por exemplo:

```bash
./build/oxente --check examples/invalid/semantico-tipo.oxe
# stderr: oxente [semântico]: examples/invalid/semantico-tipo.oxe:2:1: erro 'misturou_as_coisas': ...
# código de saída: 1
```

`--dump-ast` mostra um nó por linha, e a indentação indica os filhos. Um `.oxe` com
`inteiro r bota 2 + 3 * 4` gera:

```text
Programa
  Bloco
    Declaracao inteiro r
      Binario +
        Literal inteiro 2
        Binario *
          Literal inteiro 3
          Literal inteiro 4
```

## Código gerado

```bash
./build/oxente --emit-python examples/valid/calculo.oxe -o build/calculo.py
python3 build/calculo.py
# Resultado 14
# Com parenteses 20
```

Sem `-o`, o Python vai para stdout. O `.py` gerado já contém o runtime, então roda
sozinho, sem `PYTHONPATH` nem outros arquivos. Só programas válidos geram saída, e um
arquivo de saída já existente é preservado quando o programa tem erro.
