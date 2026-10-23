# TTT DLC Rebuild V2.0.2 — Fix real de falta de memória nas texturas

## Diagnóstico recebido em 21/10/2026

- `PORTRAITS bits=0x7` — 3 retratos OK.
- `SKIN=0x1` — fundo 4444 carregado, fonte não.
- `SKIN_BG stage=0`, 128 KiB carregados; PSP block ID `0x15D`.
- `SKIN_FONT stage=3`, PSP erro `0x800200D9` para segunda alocação de 64 KiB.
- `primitives=0x4B0` e `skipped=0xF1`: interface de emergência descartou 241 elementos por quadro.
- `assets12=0xFFF`: 12 índices CHR observados válidos, sem alteração nesta versão.

## Mudança principal

**SEM aumentar a memória estática do plugin e SEM mexer na ISO**: o renderizador aloca somente um bloco de 128 KiB (`menu_bg.4444`). Compacta o fundo para 256x160 pixels dentro desse bloco e carrega os 96 pixels superiores do atlas `font_atlas.4444` nas linhas 160 a 255 restantes. A fonte usa as células 16x16 que contêm os 96 caracteres ASCII relevantes; as 32 últimas linhas do arquivo de fonte não são necessárias.

O plugin passa a usar os mesmos 128 KiB para as **duas** texturas PSP ABGR4444, sem tentar alocar mais 64 KiB. As fontes passam a ser desenhadas por sprites (1 por caractere), não por centenas de retângulos por palavra. O modo de emergência ainda existe se houver erro real de I/O ou se nem os 128 KiB estiverem disponíveis.

## Instalação

1. Faça backup do plugin que consegue inicializar e do seu PACKFILE/ISO; não altere nenhum arquivo da ISO.
2. Extraia o ZIP completo na **raiz** do repositório GitHub, substituindo código e workflow antigos. O ZIP de atualização funciona sobre a V2.0.1.
3. Compile no GitHub Actions; novo artefato esperado: `TTT-Expansion-PRX-REBUILD-V2002`.
4. Instale a pasta **TTT_EXPANSION inteira** (PRX + texturas `.4444` + retratos) na pasta `PSP/PLUGINS/` do Memory Stick de seu PPSSPP.
5. Feche completamente o emulador e reinicie o jogo sem save state antigo.
6. Abra o menu via L+R+Quadrado. Teste aba Gokus, missão e navegação 1P..4P.

## Resultado esperado em diagnostico.txt

```text
[TTT DLC V2.0.2] PORTRAITS bits=0x00000007 ... SKIN=0x00000003
[TTT DLC V2.0.2] SKIN_BG stage=0x00000000 ... size=0x00020000 read=0x00020000
[TTT DLC V2.0.2] SKIN_FONT stage=0x00000000 ... size=0x00010000 alloc=0x00000000 read=0x0000C000
```

`alloc=0` para a fonte é INTENCIONAL: fonte compartilha a mesma reserva do BG, não há segunda alocação. `read=0xC000` significa que as 96 linhas usadas do arquivo foram carregadas; `size=0x10000` confirma tamanho completo do arquivo em disco.

Se `SKIN=3` mas imagens ficarem ilegíveis ou cores erradas, mande captura e novo log. A conversão vertical do fundo é experimental e não foi validada ainda dentro do PPSSPP.

## O que não muda

Continua PACKFILE `0x3734`; 3 IDs, 4 Colors por Goku, SFX reservados e os seis desafios manuais. Os modos História/Mapas integrados ao jogo original ainda não existem; não prometemos funcionalidade inexistente.
