# TTT Expansion PRX V0.5.2 — interceptação das chamadas diretas

Esta versão é EXPERIMENTAL e destinada somente a **Dragon Ball Z: Tenkaichi Tag Team americano, ULUS10537**, em PPSSPP. Não altera a ISO nem o PACKFILE.

## O que mudou

A V0.5/V0.5.1 tentou modificar `08A66A84`, onde o EBOOT em disco contém `jr ra` (`03E00008`). O log V0.5.1 mostrou nesse local **`6801AFE2`**, aparentemente já reescrito pelo ambiente de execução, causando o erro `0xFFFFFFF4` (-12). Por segurança, **a V0.5.2 não escreve em `08A66A84`**.

Em vez disso, a V0.5.2 procura e valida três chamadas MIPS diretas que apontam para essa função:

- `08A63A48` — `0E299AA1` (JAL `08A66A84`)
- `08A63B10` — `0E299AA1`
- `08A6408C` — `0E299AA1`

Se TODAS as três instruções corresponderem ao código esperado, troca somente esses três JALs por chamadas ao nosso interceptor, que executa a operação normal `sceDisplaySetFrameBuf` via import próprio do plugin, além do teste gráfico. Ao desligar com **L + R + Quadrado**, restaura as instruções originais. Não substitui nem modifica a importação do jogo. O painel gráfico ainda pode ser invisível ou causar glitches: a ordem dos comandos GE ainda precisa ser validada.

## Instalação

1. Substitua/adicione os arquivos desta versão nos mesmos caminhos do seu repositório GitHub. Mantenha os diretórios `src/` e `.github/workflows/`.
2. Faça commit e execute **Actions → Compilar PRX de diagnostico → Run workflow**.
3. Baixe o artifact **`TTT-Expansion-PRX-V052`**. Ele contém a pasta `TTT_EXPANSION` com `ttt_expansion.prx` e `plugin.ini`.
4. Feche o PPSSPP. Coloque a pasta em `PSP/PLUGINS/TTT_EXPANSION`, substituindo o plugin anterior. Habilite plugins nas configurações.
5. Abra o jogo desde o início, **sem savestate**. Na seleção de personagens, pressione **L + R + Quadrado** UMA VEZ.

O ideal é testar sem progresso não salvo. Se houver travamento ou glitches, feche o PPSSPP e restaure a versão anterior ou desative este plugin.

## O que registrar

Abra `PSP/PLUGINS/TTT_EXPANSION/diagnostico.txt` e localize as linhas com `[TTT V0.5.2]`:

- `CALLS APOS_START 08A63A48=... 08A63B10=... 08A6408C=... interceptor=...`
- `CALLS INSTALADO ...` ou `HOOK INSTALAR ERRO=...`.
- `HOOK installed=... calls=... draws=... buffer=... gpu_rc=...`
- Se o hook funcionar, aguarde poucos segundos para que `calls` e `draws` aumentem.

### Interpretação rápida

- `HOOK INSTALAR ERRO=0xFFFFFFF4` = primeira chamada direta não é o JAL esperado.
- `HOOK INSTALAR ERRO=0xFFFFFF90` = segunda chamada não corresponde (erro -112).
- `HOOK INSTALAR ERRO=0xFFFFFF2C` = terceira chamada não corresponde (erro -212).
- `installed=1` e `calls=0`: o jogo não percorreu essas três chamadas durante a cena; será necessário achar a chamada indireta ou outro caminho.
- `installed=1`, `calls>0`, `draws=0`: o interceptor está sendo chamado mas não encontra condição válida para desenhar.
- `draws>0`, imagem ausente: a GE desenhou, mas ainda está fora do momento de apresentação ou foi sobrescrita.

**Importante:** `0x6801AFE2` não é uma instrução de função comum que devamos sobrescrever às cegas. A V0.5.2 deliberadamente deixa esse word como está.

## Verificações locais

`cc -std=c11 -Wall -Wextra -Isrc tests/test_callsite_plan.c src/callsite_plan.c -o test_callsite && ./test_callsite`

`cc -std=gnu11 -Wall -Wextra -Isrc tests/test_native.c src/toggle.c src/overlay.c src/framecheck.c -o test_native && ./test_native`

Os testes locais verificam a geração do JAL e a recusa de instruções incompatíveis, além dos controles e buffers. A compilação real PSP e a execução dentro do PPSSPP ainda devem ser feitas no GitHub Actions e no emulador.
