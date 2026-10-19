# V0.9.8 — Protocolo de testes 1P–4P

Versão experimental. **O único personagem automaticamente suportado é Goku Early ID `00`, Color 1**, com CHR da entrada 1194 da ISO.

| Teste | Configuração no Expansion | Configuração no menu original | Log desejado |
|---|---|---|---|
| A | 1P Early / Color 1 | 1P seleciona outro personagem | `mask=1`, `ACTOR INJECT SLOT player=1 hits>=1` |
| B | **somente** 2P Early / Color 1 | Luta com 2P ativo, idealmente 2x2 | `mask=2`, `ACTOR INJECT SLOT player=2 hits>=1` |
| C | **somente** 3P Early / Color 1 | Luta com 3P ativo | `mask=4`, `ACTOR INJECT SLOT player=3 hits>=1` |
| D | **somente** 4P Early / Color 1 | Luta com 4P ativo | `mask=8`, `ACTOR INJECT SLOT player=4 hits>=1` |
| E | múltiplos slots Early / Color 1 | Luta 2x2 | `mask` com bits combinados, hits individuais |

Faça os testes A–D em execuções separadas do PPSSPP. E só avance para E se estiverem estáveis.

Se um slot estiver vazio na configuração original, o shim deixa intacto o ID original e pode informar `hits=0` — isso é uma proteção, não necessariamente uma falha.

Compare visualmente a transformação, os golpes e as animações. Verifique o cenário e a capacidade de terminar a partida, como no teste aprovado da V0.9.7.

Se houver crash ou comportamento anormal, restaure a V0.9.7 e envie o log. O código de logs não faz I/O de dentro do hook Assembly.
