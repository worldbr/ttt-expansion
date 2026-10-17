# TTT Expansion PRX V0.5.1 — diagnóstico da importação

**Jogo:** Dragon Ball Z: Tenkaichi Tag Team ULUS10537 (PPSSPP). **Atalho:** L + R + Quadrado.

## Por que esta versão?

A V0.5 mostrou `HOOK INSTALAR ERRO=0xFFFFFFF4`: o instalador não encontrou a instrução MIPS `jr ra` (`0x03E00008`) no endereço de importação `08A66A84` no momento da tentativa. O EBOOT.OLD original contém esse opcode nesse endereço em disco. A causa dessa divergência em tempo de execução ainda é desconhecida.

Esta versão registra os quatro words MIPS de `08A66A80`, `08A66A84`, `08A66A88` e `08A66A8C` em cinco momentos: `APOS_START`, `ANTES_GPU`, `DEPOIS_GPU`, `ANTES_HOOK` e `FALHA_HOOK`. O código **não remove a validação** nem procura outro lugar para modificar automaticamente. Se o stub for inesperado, nada é alterado.

## Como usar

1. Atualize seu repositório com os arquivos da V0.5.1, mantendo as pastas e faça commit.
2. Execute `Actions` > `Compilar PRX de diagnostico`; baixe o artifact `TTT-Expansion-PRX-V051`.
3. Instale o novo `ttt_expansion.prx` em `PSP/PLUGINS/TTT_EXPANSION` e reinicie o PPSSPP.
4. Entre na seleção de personagens e pressione `L + R + Quadrado` **uma vez**.
5. Envie as linhas `STUB` e `HOOK INSTALAR ERRO` do arquivo `diagnostico.txt`.

**ATENÇÃO:** caso a assinatura coincida, o código ainda inclui o hook experimental da V0.5 e poderá instalá-lo. Faça backup e não teste com progresso não salvo. Remova o PRX se o jogo travar.

O `.PRX` não foi compilado neste ambiente porque aqui não há PSPSDK. Use o workflow do GitHub Actions.
