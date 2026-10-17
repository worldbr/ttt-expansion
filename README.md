# TTT Expansion PRX V0.5.3 — teste de interface persistente

**Versão experimental para PPSSPP / Dragon Ball Z: Tenkaichi Tag Team ULUS10537.**

## Correção

Os testes anteriores comprovaram que o hook por três `jal` funciona:

- `08A63A48`, `08A63B10`, `08A6408C` mudaram para JAL do interceptor.
- `calls` aumentou até `0x223`, `draws` até `0xB4` (180).
- A V0.5.2 parava de desenhar automaticamente ao chegar a 180 desenhos, ainda que `installed=1`.
- Fechar o menu com **L + R + Quadrado** restaurou todos os JALs originais.

**Nesta V0.5.3 removemos apenas o limite de 180 desenhos.** Ela ainda desenha no máximo **uma vez por `sceDisplayGetVcount()`**, apenas com o menu aberto e hook instalado. Erros da GPU (`gpu_rc<0`) continuam desativando o desenho. Não adicionamos os personagens extras nem a interface V0.2 nesta atualização para isolar o efeito da sincronização.

## Instalação via GitHub

1. Use o pacote *ATUALIZACAO* para substituir `src/main.c`, `src/frame_hook.c` e `.github/workflows/compilar-prx.yml`, mantendo os caminhos.
2. Faça commit, abra **Actions > Compilar PRX de diagnostico > Run workflow**.
3. Baixe `TTT-Expansion-PRX-V053`, descompacte e copie `TTT_EXPANSION/` para `PSP/PLUGINS/`.
4. Feche e reabra o PPSSPP, inicie ULUS10537 normalmente (sem savestate).
5. Na tela de seleção pressione **L + R + Quadrado** e espere **10 a 15 segundos**. Pressione novamente para fechar.

## Resultado esperado

No `PSP/PLUGINS/TTT_EXPANSION/diagnostico.txt` procure as linhas `[TTT V0.5.3] HOOK`:

```
HOOK installed=0x00000001 calls=... draws=0x00000100 ... enabled=0x00000001
```

`draws` deverá ultrapassar `0xB4` (180) e `enabled` deverá permanecer 1 até você fechar. Quando fechar, o código original das três chamadas deverá ser restaurado.

**Observação:** Isso confirma a continuidade da execução, não garante que os retângulos permaneçam visíveis no menu normal do jogo. Envie uma captura na seleção com o painel aberto e o registro de `draws` após 10 segundos.

## Riscos

Esta versão modifica três instruções na RAM do jogo e envia uma lista GU repetidamente dentro da rotina de apresentação. Pode provocar instabilidade, perda de desempenho ou travamento. Não use com progresso não salvo. Se ocorrer problema, feche o PPSSPP e remova o plugin V0.5.3. Não altera a ISO/PACKFILE.

## Testes locais

```bash
cc -std=c11 -Wall -Wextra -Isrc tests/test_callsite_plan.c src/callsite_plan.c -o test_callsite && ./test_callsite
cc -std=gnu11 -Wall -Wextra -Isrc tests/test_native.c src/toggle.c src/overlay.c src/framecheck.c -o test_native && ./test_native
```

O ambiente do assistente não possui PSPSDK; a compilação PSP deve ocorrer pelo GitHub Actions.
