#!/usr/bin/env python3
"""TTT Expansion V0.9.7 — resumo offline dos eventos ACTOR CHANGE.

Uso: python tools/analisar_timeline.py diagnostico.txt
Sem bibliotecas externas. Não altera arquivos, PSP nem ISO.
"""
from pathlib import Path
import re
import sys

TAG = "[TTT V0.9.7]"
FIELD = re.compile(r"(\w+)=0x([0-9A-Fa-f]{1,8})")


def run(path: Path) -> int:
    if not path.is_file():
        print(f"Arquivo nao encontrado: {path}", file=sys.stderr)
        return 2
    text = path.read_text(encoding="utf-8", errors="replace")
    # O diagnostico e append-only. Analise somente a ultima sessao V0.9.7.
    anchor = f"{TAG} module_start"
    index = text.rfind(anchor)
    if index != -1:
        text = text[index:]
    changes = []
    for line in text.splitlines():
        if not line.startswith(TAG):
            continue
        if any(flag in line for flag in ("PLAN CONFIRMAR", "BRIDGE CONFIRMAR", "RESOURCE MATCH", "ACTOR INJECT CONFIRMAR", "ACTOR INJECT STATUS")):
            print(line)
        if "ACTOR GAP" in line:
            print("AVISO:", line)
        if "ACTOR CHANGE" in line:
            fields = {name: int(number, 16) for name, number in FIELD.findall(line)}
            if fields:
                changes.append(fields)
    if not changes:
        print("Nenhum ACTOR CHANGE encontrado na sessao selecionada.")
        return 0
    print(f"\nTimeline de atores: {len(changes)} evento(s)")
    for e in changes:
        initial = "inicio" if e.get("initial") else "mudanca"
        print(f"frame={e.get('frame', 0):>6}  {e.get('player',0)}P "
              f"{initial:>7}  ID {e.get('old_id',0):02X}->{e.get('id',0):02X}  "
              f"Color {e.get('old_color',0):X}->{e.get('color',0):X}  "
              f"plan=0x{e.get('plan',0):X} menu={e.get('menu',0)}")
    print("\nObservacao: leitura por quadro NAO revela o PC que escreve o ID.")
    print("Para encontrar a instrucao, use breakpoint de escrita no PPSSPP.")
    return 0


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Uso: python tools/analisar_timeline.py diagnostico.txt")
        raise SystemExit(2)
    raise SystemExit(run(Path(sys.argv[1])))
