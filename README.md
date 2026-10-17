# TTT Expansion PRX V0.7 — Goku Early e Goku Mid

**Base: V0.6.1 confirmada pelo usuário como funcional (overlay + bloqueio de comandos).**

## Duas entradas NOVAS, separadas

- `GOKU EARLY` = Goku (Early), era Saiyajin de Dragon Ball Z (adulto), não o Goku criança.
- `GOKU MID` = Goku (Mid), era Namek/Freeza de Dragon Ball Z (adulto).

**Importante:** a interface mostra as duas entradas independentes; o PRX ainda NÃO criou no motor do jogo dois IDs extras, novos CHR ou golpes. Não se deve confundir com os Goku Base/SSJ1 originais. IDs permanecem **TBD / pendentes de testes**. O ID `A0` experimental continua validado no projeto antigo, mas a V0.7 NÃO o atribui silenciosamente a um ou aos dois personagens. Pressionar X só gera registro no diagnostico.txt.

## Retratos ilustrativos — substituíveis

Incluí retratos de exemplo produzidos a partir de arte ilustrativa para facilitar o primeiro teste. Eles não são recursos oficiais do Tenkaichi 3 e podem ser substituídos.

- `TTT_EXPANSION/portraits/goku_early.rgba` (16.384 bytes, 64×64 RGBA8888)
- `TTT_EXPANSION/portraits/goku_mid.rgba` (16.384 bytes, 64×64 RGBA8888)
- PNGs de referência correspondentes, para inspeção e edição.

O PRX carrega os arquivos `.rgba` na abertura inicial, guarda as texturas em buffers estáveis e as desenha pela GE no seletor. Se um arquivo não for encontrado, mostra a letra E/M em vez da imagem. Log `PORTRAITS bits=0x00000003` significa que ambos os retratos foram lidos (não prova renderização visual).

Para trocar as imagens use `py -m pip install pillow` e depois:

```sh
py tools/converter_retratos.py nova_imagem_early.png TTT_EXPANSION/portraits/goku_early.rgba
py tools/converter_retratos.py nova_imagem_mid.png TTT_EXPANSION/portraits/goku_mid.rgba
```

Copie os `.rgba` resultantes para a pasta `PSP/PLUGINS/TTT_EXPANSION/portraits/` do PPSSPP e reinicie o jogo. Não precisa recompilar o PRX **para trocar só retratos**.

## Como compilar

1. Faça backup do PRX da V0.6.1.
2. Substitua os arquivos do projeto no GitHub, preservando pastas `src/`, `.github/workflows/`, `TTT_EXPANSION/` e `tools/`.
3. Faça commit, Actions → Compilar PRX do menu → Run workflow.
4. Baixe `TTT-Expansion-PRX-V070`; dentro haverá `TTT_EXPANSION/ttt_expansion.prx`, `plugin.ini` e `portraits/*.rgba`.
5. Copie **a pasta inteira TTT_EXPANSION** para `PSP/PLUGINS/`, habilite plugins e reinicie completamente o PPSSPP.
6. L + R + Quadrado para abrir/fechar. Setas selecionam personagem e Color; X registra a prévia; Círculo fecha.

## O que não mudou

- Hook de apresentação GE pelos JALs `08A63A48`, `08A63B10`, `08A6408C`.
- Bloqueio de controles na chamada em `0881A158` enquanto o menu está aberto.
- Suporte a português, inglês e espanhol. Não há alteração no EBOOT/ISO ou no PACKFILE.

## Segurança

Os hooks modificam instruções em RAM, podem gerar glitches/travamentos, e foram verificados apenas com a versão de ULUS10537 em uso nos testes anteriores. Esta versão tem **testes locais de controle e assets, mas não foi compilada com PSPSDK nem executada no PPSSPP aqui**. Mantenha a V0.6.1 funcional como backup.
