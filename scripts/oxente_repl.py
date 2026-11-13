#!/usr/bin/env python3
"""
Prompt Interativo (REPL) para Oxente Lang
Permite digitar códigos em Oxente Lang, compilar em tempo real e ver a transcrição em Python 3 limpo ou a AST.
"""

import os
import sys
import re
import subprocess
import tempfile
from pathlib import Path

ROOT_DIR = Path(__file__).resolve().parent.parent
OXENTE_EXE = ROOT_DIR / "build" / "oxente.exe"
if not OXENTE_EXE.exists():
    OXENTE_EXE = ROOT_DIR / "build" / "Debug" / "oxente.exe"

BANNER = r"""
======================================================
         Oxente Lang - Prompt Interativo (REPL)
  Escreva código Oxente Lang e veja o Python correspondente!
======================================================
Comandos especiais:
  :ast        -> Mostra a árvore sintática (AST) do último código
  :exec       -> Executa o código Python gerado
  :cru        -> Alterna exibição do código completo com runtime
  :limpar     -> Limpa o código digitado
  :sair       -> Encerra o REPL
  (Pressione ENTER numa linha vazia para compilar o bloco)
------------------------------------------------------
"""

def limpar_codigo_python(codigo_completo: str) -> str:
    linhas = codigo_completo.splitlines()
    linhas_usuario = []
    dentro_de_funcao = False

    for linha in linhas:
        linha_strip = linha.strip()
        if linha_strip.startswith("# Gerado pelo tradutor") or linha_strip.startswith("import sys") or linha_strip.startswith("import math"):
            continue
        if linha.startswith("def _ox_"):
            dentro_de_funcao = True
            continue
        if dentro_de_funcao:
            if linha.startswith("def _ox_"):
                continue
            elif linha.startswith(" ") or linha.startswith("\t") or linha_strip == "" or linha_strip.startswith("#"):
                continue
            else:
                dentro_de_funcao = False

        if linha_strip.startswith("#"):
            continue

        if linha_strip:
            linhas_usuario.append(linha)

    codigo_limpo = "\n".join(linhas_usuario)
    codigo_limpo = re.sub(r'\b_ox_desembucha\b', 'print', codigo_limpo)
    codigo_limpo = re.sub(r'\bv\d+_([a-zA-Z_][a-zA-Z0-9_]*)\b', r'\1', codigo_limpo)
    codigo_limpo = re.sub(r'_ox_div\(([^,]+),\s*([^)]+)\)', r'(\1 / \2)', codigo_limpo)
    codigo_limpo = re.sub(r'_ox_div_int\(([^,]+),\s*([^)]+)\)', r'(\1 // \2)', codigo_limpo)
    codigo_limpo = re.sub(r'_ox_mod\(([^,]+),\s*([^)]+)\)', r'(\1 % \2)', codigo_limpo)
    codigo_limpo = re.sub(r'_ox_pot\(([^,]+),\s*([^)]+)\)', r'(\1 ** \2)', codigo_limpo)
    codigo_limpo = re.sub(r'_ox_diga_ai\([^)]*\)', r'input()', codigo_limpo)
    return codigo_limpo.strip()

def compilar(codigo_oxe: str, modo: str = "--emit-python"):
    if not OXENTE_EXE.exists():
        return False, f"Executável oxente não encontrado em: {OXENTE_EXE}. Compile o projeto com cmake primeiro."

    with tempfile.NamedTemporaryFile(mode="w", suffix=".oxe", delete=False, encoding="utf-8") as f:
        f.write(codigo_oxe)
        temp_path = f.name

    try:
        resultado = subprocess.run(
            [str(OXENTE_EXE), modo, temp_path],
            capture_output=True,
            text=True,
            encoding="utf-8"
        )
        if resultado.returncode == 0:
            return True, resultado.stdout
        else:
            erro = resultado.stderr if resultado.stderr else resultado.stdout
            return False, erro.strip()
    finally:
        try:
            os.remove(temp_path)
        except OSError:
            pass

def main():
    print(BANNER)
    ultimo_codigo_oxe = ""
    ultimo_python_completo = ""
    mostrar_cru = False

    while True:
        linhas = []
        prompt = "oxente> "
        try:
            while True:
                linha = input(prompt)
                
                # Comandos imediatos se for a primeira linha
                if not linhas and linha.strip().startswith(":"):
                    cmd = linha.strip()
                    if cmd == ":sair":
                        print("Até mais!")
                        return
                    elif cmd == ":limpar":
                        linhas.clear()
                        prompt = "oxente> "
                        print("Código limpo.")
                        break
                    elif cmd == ":cru":
                        mostrar_cru = not mostrar_cru
                        estado = "ativado (mostra código com runtime completo)" if mostrar_cru else "desativado (mostra Python puro)"
                        print(f"[Modo cru {estado}]")
                        break
                    elif cmd == ":exec":
                        if not ultimo_python_completo:
                            print("[Aviso] Nenhum código Python compilado recentemente para executar.")
                        else:
                            print("\n--- Executando Programa ---")
                            subprocess.run([sys.executable, "-c", ultimo_python_completo])
                            print("---------------------------\n")
                        break
                    elif cmd == ":ast":
                        if not ultimo_codigo_oxe:
                            print("[Aviso] Nenhum código digitado ainda para exibir a AST. Digite um código primeiro!")
                        else:
                            sucesso_ast, saida_ast = compilar(ultimo_codigo_oxe, modo="--dump-ast")
                            print("\n🌳 Árvore Sintática Abstrata (AST):")
                            print("-" * 40)
                            print(saida_ast.strip())
                            print("-" * 40 + "\n")
                        break
                    else:
                        print(f"Comando desconhecido: {cmd}")
                        break

                # Se apertar Enter em uma linha vazia após digitar código, compila o bloco
                if linha.strip() == "" and linhas:
                    break

                linhas.append(linha)
                prompt = "   ...  "

        except (KeyboardInterrupt, EOFError):
            print("\nSaindo...")
            break

        codigo = "\n".join(linhas).strip()
        if not codigo:
            continue

        ultimo_codigo_oxe = codigo

        print("\n Compilando e traduzindo...")
        sucesso, saida = compilar(codigo, modo="--emit-python")

        if sucesso:
            ultimo_python_completo = saida
            print("\Sucesso! Código Python:")
            print("-" * 40)
            if mostrar_cru:
                print(saida.strip())
            else:
                limpo = limpar_codigo_python(saida)
                print(limpo if limpo else saida.strip())
            print("-" * 40)
            print("Dica: digite ':ast' para ver a árvore ou ':exec' para rodar.\n")
        else:
            print("\Erro encontrado no código:")
            print("-" * 40)
            print(saida)
            print("-" * 40 + "\n")

if __name__ == "__main__":
    main()
