# TTT Expansion V0.5 — teste de hook em sceDisplaySetFrameBuf

**JOGO:** DBZ Tenkaichi Tag Team ULUS10537 (PPSSPP). **ATALHO:** L + R + Quadrado. **STATUS:** código-fonte experimental; compilação no PSPSDK e funcionamento no PPSSPP AINDA NÃO VERIFICADOS.

## O que o debugger confirmou

- O EBOOT importa `sceDisplaySetFrameBuf` em `0x08A66A84` (o breakpoint disparou).
- O jogo forneceu `a0=0x04088000`, `a1=512`, `a2=3` (RGBA8888), `a3=0` (NEXTHSYNC).
- A V0.4 desenha durante telas pretas, mas fica encoberta durante o jogo; a V0.5 testa desenho no momento da chamada do SetFrameBuf.

## O que o plugin faz

1. Carrega normalmente (com diagnóstico escrito no arquivo).
2. Quando L+R+Quadrado é pressionado, inicia a GE e verifica se `0x08A66A84` contém a importação original (`jr ra` mais `syscall` ou `nop`).
3. Substitui temporariamente as duas instruções da importação por um salto MIPS a uma função própria (e invalida os caches).
4. A função interceptadora executa um teste gráfico somente com framebuffer válido, no máximo **180 quadros**, e sempre encaminha a chamada à `sceDisplaySetFrameBuf` importada pelo próprio plugin.
5. Ao pressionar novamente o atalho, restaura as instruções originais. O plugin não altera os arquivos da ISO nem a identidade dos personagens.

**RISCO:** o hook modifica código de execução do jogo em RAM e pode não ser compatível com outra região/patch. Mesmo que a compilação funcione, a injeção pode travar o jogo ou causar falhas visuais. Use apenas no PPSSPP, em cópia do jogo e sem dados não salvos. Se travar, feche o PPSSPP, remova o PRX V0.5 e restaure V0.4. Não utilize savestate antigo como primeiro teste.

## Instalação e teste

1. Atualize seu repositório GitHub com os arquivos deste ZIP, incluindo `.github/workflows/compilar-prx.yml`.
2. Em **Actions**, execute **Compilar PRX de diagnostico**. Artifact esperado: `TTT-Expansion-PRX-V050`. Se der erro, obtenha `TTT-Expansion-Build-Log-V050`.
3. Copie `ttt_expansion.prx` e `plugin.ini` para `PSP/PLUGINS/TTT_EXPANSION/`, substituindo a versão anterior, e **reinicie completamente** o PPSSPP.
4. Na seleção de personagens, pressione L+R+Quadrado uma vez. Espere de 2 a 5 segundos. Veja se aparecem os retângulos azul e amarelo DURANTE a cena (não apenas na tela preta).
5. Pressione o atalho novamente para fechar/restaurar o hook.
6. Envie as linhas do `diagnostico.txt` com `HOOK INSTALADO` ou `HOOK INSTALAR ERRO` e as linhas `HOOK installed=... calls=... draws=... gpu_rc=...`.

## Como interpretar o log

- `HOOK INSTALADO` + `calls>0` e `draws>0`: a importação em `08A66A84` foi redirecionada, e nosso desenho foi executado na chamada de apresentação (mas pode continuar invisível).
- `calls=0`: o jogo não chamou a importação interceptada após a instalação ou o PPSSPP não atualizou o código JIT.
- `HOOK INSTALAR ERRO=0xFFFFFFF4` (-12): a instrução encontrada não corresponde a `jr ra`; nenhum patch foi aplicado.
- `gpu_rc!=0`: falha do desenho GE/GU. Conteúdo da tela e logs ajudam a separar problema de sincronização e de GPU.

**Limitações conhecidas:** esta é uma tentativa de overlay sem texto utilizando um hook experimental em user-mode; não inclui a interface V0.2 e não comprova compatibilidade com todos os backends gráficos. Um hook no SetFrameBuf pode ainda ocorrer antes que a GE termine o quadro, dependendo do pipeline do jogo.
