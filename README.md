# TTT Expansion V0.9.8.9 — menu seguro e 4 Colors para Goku Mid SSJ

Versão construída sobre a **V0.9.8.7**, não sobre a V0.9.8.8 que apresentou tela preta antes da abertura do menu.

- Restaurado o buffer gráfico do tamanho comprovado na V0.9.8.7: **128 KiB**, em vez de 512 KiB.
- Aplicado limite conservador de **1200 primitivas por quadro**, com contagem de primitivas descartadas e canários nas bordas, para evitar corrupção silenciosa dos dados do hook.
- O menu continua com três Gokus e 4 Colors por personagem: Goku Início ID00, Goku Mid ID01 e **Goku Mid SSJ ID02**.
- Goku Mid SSJ usa índices internos **`0x371B`, `0x371C`, `0x371D`, `0x371E`** (entradas 14108–14111), Colors 1–4.
- SFX do Goku Mid SSJ reservado em `0x371F` (entrada 14112), não redirecionado.
- Mantido suporte a quatro jogadores com seleção de personagem/Color independente por slot.
- Logs `GE GUARD` mostram `primitives`, `peak`, `skipped`, `guard_fault`, `list_bytes`; o esperado para `list_bytes` é `0x00020000`.

## Como testar

1. **Mantenha a ISO atual.** Não edite PACKFILE nem EBOOT.
2. Atualize os arquivos na **raiz** do repositório GitHub e execute `.github/workflows/compilar-prx.yml` (artefato esperado `TTT-Expansion-PRX-V0989`).
3. Guarde o PRX da V0.9.8.7 como backup. Instale só o novo PRX e reinicie completamente o PPSSPP; **não use save-state criado com plugin antigo**.
4. Confirme **1P Goku Início Color 1**, alterne para 2P, selecione **Goku Mid SSJ Color 1** e confirme. Depois alterne para 3P e 4P e teste as outras Colors.
5. Se a tela ficar preta, restaure a V0.9.8.7. Envie o começo do `diagnostico.txt` e o erro do PPSSPP; não continue tentando com a mesma ISO como se ela estivesse danificada.
6. Se o menu sumir, envie `MENU WATCHDOG`, `GE GUARD` e `HOOK`. Se `skipped` subir, o limite de primitivas impediu novos desenhos e ajustaremos o renderizador sem aumentar a memória estática.

**Limites:** sem compilador PSPSDK disponível aqui; testes nativos não comprovam inicialização e renderização no PPSSPP. A assinatura do novo buffer e o orçamento foram testados estaticamente.
