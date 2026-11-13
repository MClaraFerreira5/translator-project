#!/usr/bin/env python3
"""
Prompt Interativo (REPL) para Oxente Lang
Permite digitar códigos em Oxente Lang, compilar em tempo real e ver a transcrição em Python 3 limpo.
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
  :exec       -> Executa o código gerado
  :cru        -> Alterna exibição do código completo (com runtime interno)
  :ast        -> Mostra a árvore sintática (AST) do código
  :limpar     -> Limpa o código digitado
  :sair       -> Encerra o REPL
  (Pressione ENTER numa linha vazia para compilar o bloco)
------------------------------------------------------
"""

def limpar_codigo_python(codigo_completo: str) -> str:
    """
    Remove todo o boilerplate/runtime interno e converte
    v1_var para var e _ox_desembucha(...) para print(...).
    """
    linhas = codigo_completo.splitlines()
    linhas_usuario = []
    
    # Ignora importações iniciais, comentários de cabeçalho e definições de funções auxiliares _ox_*
    dentro_de_funcao = False
    for linha in linhas:
        linha_strip = linha.strip()
        
        # Ignora cabeçalho gerado e imports
        if linha_strip.startswith("# Gerado pelo tradutor") or linha_strip.startswith("import sys") or linha_strip.startswith("import math"):
            continue
            
        # Detecta início de funções de suporte _ox_*
        if linha.startswith("def _ox_"):
            dentro_de_funcao = True
            continue
            
        # Se estamos dentro de uma função auxiliar de runtime
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

    # 1. Troca _ox_desembucha(...) por print(...)
    codigo_limpo = re.sub(r'\b_ox_desembucha\b', 'print', codigo_limpo)

    # 2. Converte variáveis renomeadas pelo gerador (ex: v1_a -> a, v2_soma -> soma)
    codigo_limpo = re.sub(r'\bv\d+_([a-zA-Z_][a-zA-Z0-9_]*)\b', r'\1', codigo_limpo)

    # 3. Simplifica chamadas de runtime aritmético comuns para operadores normais se houver
    # Ex: _ox_soma(a, b) -> a + b (caso ocorram)
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
                            print("Nenhum código Python compilado recentemente para executar.")
                        else:
                            print("\n--- Executando Programa ---")
                            subprocess.run([sys.executable, "-c", ultimo_python_completo])
                            print("---------------------------\n")
                        break
                    elif cmd == ":ast":
                        print("[Modo AST: digite o código abaixo e aperte Enter duas vezes]")
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

        sucesso, saida = compilar(codigo, modo="--emit-python")

        if sucesso:
            ultimo_python_completo = saida
            print("Código Python:")
            print("-" * 40)
            if mostrar_cru:
                print(saida.strip())
            else:
                limpo = limpar_codigo_python(saida)
                print(limpo if limpo else saida.strip())
            print("-" * 40)
        else:
            print("Erro encontrado:")
            print("-" * 40)
            print(saida)
            print("-" * 40 + "\n")

if __name__ == "__main__":
    main()
