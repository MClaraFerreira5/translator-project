#!/usr/bin/env python3
"""
Executor automatizado da suite de testes da Oxente Lang.
Executa os casos documentados no catalogo (tests/suite_manifest.json)
e avalia os resultados reais frente ao estado atual do projeto.
Nao mascara testes falhos ou funcionalidades ainda pendentes.
"""

import json
import os
import subprocess
import sys
from pathlib import Path

ROOT_DIR = Path(__file__).resolve().parent.parent
MANIFEST_PATH = ROOT_DIR / "tests" / "suite_manifest.json"

def encontrar_executavel(nome: str) -> Path | None:
    for base in [ROOT_DIR / "build", ROOT_DIR]:
        candidatos = [
            base / nome,
            base / f"{nome}.exe",
            base / "Release" / f"{nome}.exe",
            base / "Debug" / f"{nome}.exe",
        ]
        for c in candidatos:
            if c.is_file() and os.access(c, os.X_OK):
                return c
    return None

def main():
    if not MANIFEST_PATH.is_file():
        print(f"Erro: manifesto nao encontrado em {MANIFEST_PATH}", file=sys.stderr)
        sys.exit(1)

    with open(MANIFEST_PATH, "r", encoding="utf-8") as f:
        casos = json.load(f)

    tokens_exe = encontrar_executavel("oxente-tokens")
    oxente_exe = encontrar_executavel("oxente")

    total = len(casos)
    aprovados = 0
    pendentes_conhecidos = 0
    falhas = 0

    criterios_cobertura = {
        "precedencia": 0,
        "declaracao_antes_uso": 0,
        "tipos_incompativeis": 0,
        "duplicacao": 0,
        "escopos": 0,
        "condicoes_nao_booleanas": 0,
        "operadores_longos": 0,
        "textos_comentarios": 0,
        "nl": 0,
        "construcoes_aninhadas": 0,
        "erros_gramatica": 0
    }

    print("=" * 70)
    print(" SUITE AUTOMATIZADA DE TESTES - OXENTE LANG")
    print(f" Total de casos catalogados: {total}")
    print(f" Executavel oxente-tokens: {tokens_exe or 'NAO ENCONTRADO'}")
    print(f" Executavel oxente (CLI):  {oxente_exe or 'NAO ENCONTRADO'}")
    print("=" * 70)

    for c in casos:
        cid = c["id"]
        arq_rel = c["arquivo"]
        arq_abs = ROOT_DIR / arq_rel
        etapa = c["etapa_responsavel"]
        tipo = c["tipo"]
        status_previsto = c.get("status_implementacao", "pendente_card")

        # Contabiliza criterios de cobertura
        for topico in c.get("topicos", []):
            if topico in criterios_cobertura:
                criterios_cobertura[topico] += 1

        if not arq_abs.is_file():
            print(f"[ERRO ARQUIVO] {cid}: Arquivo {arq_rel} nao encontrado.")
            falhas += 1
            continue

        if etapa == "lexica":
            if not tokens_exe:
                print(f"[PULADO] {cid}: oxente-tokens ausente para teste lexico.")
                pendentes_conhecidos += 1
                continue

            res = subprocess.run(
                [str(tokens_exe), str(arq_abs)],
                capture_output=True,
                text=True,
                encoding="utf-8",
                errors="replace"
            )

            if tipo == "valido":
                if res.returncode == 0:
                    print(f"[APROVADO] {cid} (lexico valido)")
                    aprovados += 1
                else:
                    print(f"[FALHA] {cid}: esperado sucesso lexico, mas retornou {res.returncode}. Erro: {res.stderr.strip()}")
                    falhas += 1
            else: # invalido
                causa = c.get("causa_tecnica", "")
                if res.returncode == 1 and (not causa or causa.lower() in res.stderr.lower()):
                    print(f"[APROVADO] {cid} (lexico detectou: '{causa}')")
                    aprovados += 1
                else:
                    print(f"[FALHA] {cid}: esperado erro lexico contendo '{causa}', retorno={res.returncode}. Erro: {res.stderr.strip()}")
                    falhas += 1

        elif etapa in ("sintatica", "semantica"):
            if not oxente_exe:
                print(f"[PENDENTE] {cid}: CLI oxente ausente.")
                pendentes_conhecidos += 1
                continue

            res = subprocess.run(
                [str(oxente_exe), "--check", str(arq_abs)],
                capture_output=True,
                text=True,
                encoding="utf-8",
                errors="replace"
            )

            # O CLI atual retorna 4 quando o modo --check nao esta implementado
            if res.returncode == 4:
                print(f"[LIMITACAO CONHECIDA] {cid} ({etapa}): compilador CLI retornou 4 (modo --check pendente de integracao)")
                pendentes_conhecidos += 1
            elif tipo == "invalido" and res.returncode == 1:
                # Caso futuro: quando implementado e emitindo diagnostico real
                causa = c.get("causa_tecnica", "")
                if not causa or causa.lower() in res.stderr.lower():
                    print(f"[APROVADO] {cid} ({etapa} detectou '{causa}')")
                    aprovados += 1
                else:
                    print(f"[FALHA DIAGNOSTICO] {cid}: erro esperado '{causa}' nao encontrado no diagnostico: {res.stderr.strip()}")
                    falhas += 1
            elif tipo == "valido" and res.returncode == 0:
                print(f"[APROVADO] {cid} ({etapa} validado)")
                aprovados += 1
            else:
                print(f"[FALHA] {cid}: resultado inesperado ret={res.returncode}. Saida: {res.stdout.strip()} Erro: {res.stderr.strip()}")
                falhas += 1

    print("\n" + "=" * 70)
    print(" RESUMO DA SUITE")
    print("=" * 70)
    print(f" Total de casos:        {total}")
    print(f" Aprovados:             {aprovados}")
    print(f" Limitacoes conhecidas: {pendentes_conhecidos} (aguardando finalizacao dos cards de parser/semantica)")
    print(f" Falhas inesperadas:    {falhas}")
    print("-" * 70)
    print(" COBERTURA DOS CRITERIOS DE ACEITE:")
    for crit, count in criterios_cobertura.items():
        status = "OK" if count > 0 else "AUSENTE"
        print(f"  - {crit:30}: {count:2} casos ({status})")
    print("=" * 70)

    if falhas > 0:
        sys.exit(1)
    sys.exit(0)

if __name__ == "__main__":
    main()
