# TTT Expansion Menu V0.6.1 — Bloqueio de controles do jogo

**PPSSPP / ULUS10537 (EBOOT.OLD testado pelo usuário).** Evolução da V0.6, preservando o menu azul/amarelo e o hook de apresentação de quadros da V0.5.3. **L + R + Quadrado** permanece o atalho.

## Implementação

- O `EBOOT.OLD` foi analisado: há um `jal 0x08A66A6C` em **`0x0881A158`**, opcode original **`0x0E299A9B`**, na rotina que chama `sceCtrlReadBufferPositive` e grava na região do pad `0x08AF31A0`.
- Na inicialização, o plugin valida essa instrução; se coincidir, troca **somente o JAL desse endereço** para um interceptador na memória do PRX. Não modifica o stub HLE do PPSSPP e preserva a chamada original pela importação independente do plugin.
- Com o menu aberto, o jogo recebe `Buttons=0`, `Lx=128`, `Ly=128`; o plugin continua usando `sceCtrlPeekBufferPositive` para os comandos REAIS.
- O atalho e o botão que fecha o menu são consumidos até **todos** os botões serem liberados; isso evita que o quadrado, círculo ou X vaze para o menu original ao fechar.
- Ao fechar o menu, o controle é restaurado. Ao encerrar a sessão do plugin, o JAL original é restaurado (desde que ainda seja nosso opcode).
- Caso a assinatura seja diferente (ou o JAL não possa alcançar o plugin), **não instala o bloqueio** e registra `INPUT INSTALAR ERRO` (sem forçar alteração indevida).
- **Nenhuma alteração ao EBOOT/ISO em disco, ao PACKFILE ou aos IDs de personagens.** A confirmação por X continua sendo somente um teste registrado no log.

## Instalação

1. Faça backup da V0.6 que já funcionou.
2. Extraia o `ATUALIZACAO.zip` para a raiz do repositório GitHub, preservando `src/`, `tests/`, `Makefile` e `.github/workflows/`.
3. Commit, Actions > Compilar PRX do menu > Run workflow. Artifact esperado: `TTT-Expansion-PRX-V061`.
4. Copie `TTT_EXPANSION` para `PSP/PLUGINS/` do PPSSPP substituindo o `ttt_expansion.prx` anterior.
5. Reinicie o PPSSPP e o jogo completamente. Na seleção de personagens pressione L+R+Quadrado; tente mover esquerda/direita e X enquanto nosso menu está aberto.
6. Verifique `diagnostico.txt`: linhas `INPUT HOOK INSTALADO` e `INPUT STATUS` com `muted` aumentando. Ao fechar, navegação do jogo deve voltar ao normal.

## Limitações e segurança

- **Primeiro teste:** interceptamos a única chamada direta `sceCtrlReadBufferPositive` encontrada no EBOOT principal. Outras rotinas, módulos carregáveis e caminhos de entrada podem existir e não foram confirmados. Se o menu original continuar se mexendo, envie logs para localizar o caminho efetivamente utilizado. Não é bloqueio universal garantido.
- O jogo pode manter uma ação já iniciada no quadro em que o atalho foi reconhecido. Há um filtro preventivo do próprio atalho, mas a sincronização de threads ainda pode gerar um quadro de diferença.
- O código altera **uma instrução MIPS na memória em execução**, podendo provocar glitches/travamento. Mantenha backup, teste com progresso salvo e remova o plugin se houver problema.
- Não sabemos se todas as revisões do ULUS10537 têm as mesmas instruções. A validação faz o plugin recusar patches incompatíveis.
- Compilação PSPSDK e comportamento PPSSPP a validar no GitHub e no emulador do usuário; aqui foram feitos apenas testes nativos e inspeção do ELF.

## Testes em PC (sem PSPSDK)

```bash
cc -std=gnu11 -Wall -Wextra -Isrc tests/test_input_guard.c src/input_guard.c src/input_plan.c -o /tmp/test_input && /tmp/test_input
cc -std=gnu11 -Wall -Wextra -Isrc tests/test_menu_state.c src/menu_state.c -o /tmp/test_menu && /tmp/test_menu
cc -std=gnu11 -Wall -Wextra -Isrc tests/test_callsite_plan.c src/callsite_plan.c -o /tmp/test_calls && /tmp/test_calls
cc -std=gnu11 -Wall -Wextra -Isrc tests/test_native.c src/toggle.c src/overlay.c src/framecheck.c -o /tmp/test_fb && /tmp/test_fb
```
