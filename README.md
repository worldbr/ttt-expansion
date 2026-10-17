# TTT Expansion Menu V0.9.2 — Teste CHR independente (ULUS10537)

Esta versão parte da **V0.9.1**, que você confirmou entrar em batalha mantendo ID externo `00` e gráficos normais. Preserva o GPU Guard, o menu visual, retratos e bloqueio de controles.

## O que muda

Ao confirmar **1P → Goku Early → Color 1 → X**, a V0.9.2 instala uma *ponte* que:

1. Preserva o ID externo `00` na seleção experimental **Videl forma 2**, conforme V0.9.1.
2. Intercepta cinco chamadas verificadas do jogo às rotinas `089C6CA0` e `089C6D60` que convertem ID do personagem + variante em ID de recurso do PACKFILE.
3. **Somente se `external_id == 0` e `resource_offset == 0`**, devolve o ID de recurso **`0x4A9`**, correspondente ao arquivo **1194 (`0x4AA` mostrado pelo Index Editor)**.
4. Para os demais pedidos, chama a rotina original e devolve seu resultado. Não altera os arquivos do Goku original (ID `03`).
5. Antes de instalar, confere os cinco opcodes `JAL`, o cabeçalho `PacK`, o total de 14097 registros e a entrada `0x4A9` do índice em memória, recusando valores incompatíveis.

**Não é Mod Loader externo**: o arquivo novo continua sendo lido pela rotina original do PACKFILE na sua ISO. A versão atual não redireciona Color 2–4 nem SFX, e continua dependendo do slot experimental da Videl. O modelo será visualmente igual ao Goku original porque você colocou uma cópia idêntica do CHR no índice 1194.

## Testar

1. Utilize a ISO que você já confirmou conter uma cópia do CHR Goku Base Color 1 na entrada **1194 do Index Editor**. Faça backup da ISO e do PRX anterior.
2. Desabilite quaisquer cheats antigos de expansão. Use o EBOOT original da versão ULUS10537; não aplique patches concorrentes nesses endereços.
3. Atualize GitHub com os arquivos do projeto (ou só o ZIP atualização), commit e execute Actions **Compilar PRX do menu**. Artifact esperado: `TTT-Expansion-PRX-V092`.
4. Instale o PRX V0.9.2, reinicie completamente o PPSSPP. Teste abrir/fechar o menu e uma batalha normal com Goku original ID03, antes de ativar a ponte.
5. Reinicie o jogo, selecione **1P → Goku Early → Color 1 → X** dentro do TTT Expansion. Verifique em `diagnostico.txt` a linha `RESOURCE CONFIRMAR` com `installed=0x00000001`, `record_id=0x000004A9` e `size_1194` > 0, e `BRIDGE CONFIRMAR installed=1`.
6. Feche o plugin com Círculo, selecione **Videl segunda forma experimental** no menu original e entre em batalha.
7. Confirme o ID de batalha do 1P em `08B48AE8 == 00`. Após a batalha, abra o overlay para ler os contadores `RESOURCE STATUS`: `redirects` deverá aumentar e `last_result` quando corresponder à primeira Color pode ser `0x4A9`. `checked`/`unchecked` contam todas as conversões de recursos interceptadas, mesmo quando não há redirecionamento.

### Quando o resultado não for o esperado

- `RESOURCE CONFIRMAR installed=0`: envie a linha completa, sobretudo `error`, `pack_magic`, `count`, `record_id`, `size_1194`. Um erro -10 a -13 indica metadados do PACKFILE inesperados; NÃO force patch nem carregamento.
- `BRIDGE CONFIRMAR installed=0`: a ponte ID00 foi recusada; o hook de recursos é desinstalado automaticamente. Envie o erro.
- Se `redirects=0` em batalha: nossas cinco chamadas interceptadas não receberam o par `(id=00,offset=0)`; precisaremos achar a rotina/caminho real do carregamento antes de considerar comprovada a independência.
- Se modelo visual igual ao Goku: esperado, pois os dados da entrada 1194 são uma cópia. Não considere comprovação visual.
- Se travamento, textura/cenário corrompidos ou erro de recursos: feche completamente o PPSSPP, restaure PRX V0.9.1 e backup da ISO se necessário.

**Quadrado sozinho**, com o overlay aberto e fora da batalha, tenta desarmar primeiro o redirecionamento e depois a ponte ID00. Não desarme durante carregamento ou batalha.

## Limites

Testes C locais não substituem compilação PSPSDK ou validação no PPSSPP. Os hooks alteram instruções executáveis em RAM: uso experimental e somente ULUS10537 com EBOOT.OLD correspondente. O tamanho do PACKFILE foi mantido em 14097 registros; são reaproveitadas entradas existentes, não criadas novas.
