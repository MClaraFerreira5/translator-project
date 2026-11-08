#!/usr/bin/env python3
"""Testes de ponta a ponta da CLI: .oxe -> .py -> execução com Python 3.

Compara stdout, stderr e código de saída. Cada programa roda em uma pasta
isolada e sem PYTHONPATH, o que confirma que o .py gerado é autossuficiente.

Uso: e2e.py <executavel-oxente> <dir-temporario>
"""
import json
import os
import pathlib
import subprocess
import sys

RAIZ = pathlib.Path(__file__).resolve().parent.parent
LIMITE = 20

EXEMPLOS = {
    "calculo": "Resultado 14\nCom parenteses 20\n",
    "decisao": "Acesso permitido\n",
    "repeticao": "15\n",
    "reais": "3,500\n2,500\n",
    "ramos": "Recuperacao\n",
    "comentarios": "primeiramente espia arriba\n",
    "operadores-longos": "3\n",
    "aninhamento": "32\n",
    "escopo-independente": "10\n20\n",
    "lacos-aninhados": "4\n",
    "nl-crlf-eof": "3\n",
    "precedencia-logica": "evera\npala\nevera\n",
    "precedencia-potencia": "512,000\n-4,000\n24,000\n",
}

# nome, código, stdin, stdout, trechos do stderr, código de saída
PROGRAMAS = [
    ("entrada_inteiro", "inteiro n bota 0\ndiga_ai(n)\ndesembucha(n + 1)\n", "41\n", "42\n", [], 0),
    ("entrada_tipos",
     "tronxo t bota 0\npapo p bota pala\nresenha r bota vei a tendeu\n"
     "diga_ai(t)\ndiga_ai(p)\ndiga_ai(r)\ndesembucha(t; p; r)\n",
     "7,5\nevera\noxente visse\n", "7,500 evera oxente visse\n", [], 0),
    ("divisao_negativa",
     "desembucha(-7 // 2)\ndesembucha(-7 % 2)\ndesembucha(7 // -2)\ndesembucha(7 % -2)\n", "",
     "-3\n-1\n-3\n1\n", [], 0),
    ("arrodeia_simbora",
     "arrodeia (inteiro i bota 1; ate i > 4; i bota i + 1) bora\n    vai_que (i % 2 == 0) bora\n"
     "        simbora\n    arremata\n    desembucha(i)\narremata\n", "", "1\n3\n", [], 0),
    ("escopos_por_bloco",
     "vai_que (evera) bora\n    inteiro x bota 1\n    desembucha(x)\narremata\n"
     "vai_que (evera) bora\n    resenha x bota vei oxe tendeu\n    desembucha(x)\narremata\n"
     "arrodeia (tronxo x bota 2; ate x > 2; x bota x + 1) bora\n    desembucha(x)\narremata\n", "",
     "1\noxe\n2,000\n", [], 0),
    ("divisao_por_zero", "inteiro a bota 1\ndesembucha(a / 0)\n", "", "", ["dividiu_por_nada", "linha 2"], 1),
    ("divisao_inteira_por_zero", "desembucha(1 // 0)\n", "", "", ["dividiu_por_nada", "linha 1"], 1),
    ("resto_por_zero", "\n\ndesembucha(1 % 0)\n", "", "", ["dividiu_por_nada", "linha 3"], 1),
    ("saida_antes_do_erro", "desembucha(1)\ndesembucha(1 / 0)\ndesembucha(2)\n", "", "1\n",
     ["dividiu_por_nada", "linha 2"], 1),
    ("entrada_invalida", "inteiro n bota 0\ndiga_ai(n)\n", "abc\n", "", ["entrada_invalida", "linha 2"], 1),
    ("entrada_ausente", "inteiro n bota 0\ndiga_ai(n)\n", "", "", ["entrada_ausente", "linha 2"], 1),
    ("potencia_invalida", "desembucha(0 ** -1)\n", "", "", ["potencia_invalida", "linha 1"], 1),
]

falhas = []
casos = 0


def rodar(comando, pasta, entrada=""):
    ambiente = {k: v for k, v in os.environ.items() if k != "PYTHONPATH"}
    ambiente["PYTHONIOENCODING"] = "utf-8"
    r = subprocess.run(comando, cwd=pasta, input=entrada, capture_output=True, text=True,
                       encoding="utf-8", errors="replace", timeout=LIMITE, env=ambiente)
    return r.returncode, r.stdout.replace("\r\n", "\n"), r.stderr.replace("\r\n", "\n")


def falhar(nome, motivo):
    falhas.append(nome)
    print(f"[FALHA] {nome}: {motivo}", file=sys.stderr)


def verificar_execucao(nome, oxe, pasta, oxente, entrada, stdout, trechos, codigo):
    global casos
    casos += 1
    pasta.mkdir(parents=True, exist_ok=True)
    py = pasta / "programa.py"
    py.unlink(missing_ok=True)

    ret, saida, erro = rodar([oxente, "--emit-python", str(oxe), "-o", str(py)], pasta)
    if ret != 0 or saida or erro or not py.is_file():
        return falhar(nome, f"traducao: codigo {ret}, stdout {saida!r}, stderr {erro!r}")

    ret, saida, erro = rodar([sys.executable, "-B", str(py)], pasta, entrada)
    if ret != codigo:
        falhar(nome, f"codigo {ret}, esperado {codigo}; stderr {erro!r}")
    if saida != stdout:
        falhar(nome, f"stdout {saida!r}, esperado {stdout!r}")
    if not trechos and erro:
        falhar(nome, f"stderr inesperado: {erro!r}")
    for trecho in trechos:
        if trecho not in erro:
            falhar(nome, f"stderr sem {trecho!r}: {erro!r}")


def verificar_invalidos(pasta, oxente):
    global casos
    manifesto = json.loads((RAIZ / "tests" / "suite_manifest.json").read_text(encoding="utf-8"))
    pasta.mkdir(parents=True, exist_ok=True)
    for caso in manifesto:
        if caso["tipo"] != "invalido":
            continue
        casos += 1
        nome, py = caso["id"], pasta / "nao_deve_existir.py"
        ret, saida, erro = rodar([oxente, "--emit-python", str(RAIZ / caso["arquivo"]), "-o", str(py)], pasta)
        if ret != 1:
            falhar(nome, f"codigo {ret}, esperado 1")
        if saida:
            falhar(nome, f"stdout inesperado: {saida!r}")
        if caso["causa_tecnica"].lower() not in erro.lower():
            falhar(nome, f"stderr sem {caso['causa_tecnica']!r}: {erro!r}")
        if py.exists():
            falhar(nome, "gerou .py para programa invalido")
            py.unlink()


def main():
    if len(sys.argv) != 3:
        print("Uso: e2e.py <executavel-oxente> <dir-temporario>", file=sys.stderr)
        return 2
    oxente = str(pathlib.Path(sys.argv[1]).resolve())
    base = pathlib.Path(sys.argv[2]).resolve()

    for nome, stdout in EXEMPLOS.items():
        oxe = RAIZ / "examples" / "valid" / f"{nome}.oxe"
        verificar_execucao(nome, oxe, base / nome, oxente, "", stdout, [], 0)

    for nome, codigo_oxe, entrada, stdout, trechos, codigo in PROGRAMAS:
        pasta = base / nome
        pasta.mkdir(parents=True, exist_ok=True)
        oxe = pasta / "programa.oxe"
        oxe.write_bytes(codigo_oxe.encode("ascii"))
        verificar_execucao(nome, oxe, pasta, oxente, entrada, stdout, trechos, codigo)

    verificar_invalidos(base / "invalidos", oxente)

    print(f"{casos - len(set(falhas))}/{casos} casos de ponta a ponta sem falha")
    return 1 if falhas else 0


if __name__ == "__main__":
    sys.exit(main())
