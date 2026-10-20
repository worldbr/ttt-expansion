# TTT Expansion V1.0.0 beta — Menu refeito com texturas de verdade

## O que é entregue

**Plugin C modificado**, não só imagem. Agora `src/gpu_probe.c` carrega e desenha uma skin por `sceGuTexImage` na GPU do PSP. O fundo é PNG HD editável convertido em arquivo de textura, e a tipografia é um atlas de sprites. Retratos e destaque da seleção continuam dinâmicos.

Ao trocar 1P, 2P, 3P ou 4P, escolher um personagem ou modificar a Color, a tela responde pelo código do plugin. Preservamos as funções de hook/bridge/injeção da V0.9.8.9.

## Como instalar — Windows/PPSSPP

1. **Faça backup** do PRX anterior funcional e de sua ISO. Não altere a ISO/PACKFILE nem o EBOOT para testar esta interface.
2. Extraia o ZIP **COMPLETO** na raiz do seu repositório GitHub (pastas `src`, `TTT_EXPANSION`, `.github`, `Makefile`, etc.). Não crie uma subpasta `TTT_Expansion_V1000` dentro do repositório.
3. No GitHub: Actions -> `Compilar PRX do menu` -> `Run workflow`.
4. Baixe o artefato **`TTT-Expansion-PRX-V1000`**. Nele já deve haver `TTT_EXPANSION/ttt_expansion.prx`, `plugin.ini`, `portraits/*.rgba` e `ui/*.4444`.
5. Copie **a pasta `TTT_EXPANSION` inteira** do artefato para a pasta de Plugins do PPSSPP (em geral `memstick/PSP/PLUGINS/TTT_EXPANSION/`), substituindo os arquivos antigos. Confirme que o PPSSPP está configurado para utilizar esse diretório de Memory Stick.
6. Habilite o plugin, **feche totalmente o PPSSPP e abra de novo**. Evite save-states criados com a versão anterior.
7. Atalho do menu: **L+R+Quadrado**. X confirma, L/R navega entre jogadores, cima/baixo troca os três personagens, esquerda/direita troca uma das quatro Colors; Triângulo muda aba; Select muda idioma; Círculo fecha.

## Arquivos das texturas

| Arquivo | Função | Formato |
|---|---|---|
| `TTT_EXPANSION/ui/menu_bg_HD.png` | Fundo original em alta resolução | PNG RGB 1440x816 |
| `TTT_EXPANSION/ui/menu_bg.4444` | Fundo que o PRX realmente carrega | Raw ABGR4444 256x256, 128 KB |
| `TTT_EXPANSION/ui/font_atlas.png` | Atlas editável de fonte | PNG RGBA 256x128 |
| `TTT_EXPANSION/ui/font_atlas.4444` | Atlas efetivamente usado | Raw ABGR4444 256x128, 64 KB |
| `TTT_EXPANSION/portraits/*.rgba` | Retratos, incluindo Mid SSJ | Raw RGBA8888 64x64 |
| `TTT_EXPANSION/ui/PREVIEW_MENU_REFEITO.png` | Referência visual, não usada pelo PRX | PNG 960x544 |

Para **mudar as texturas** posteriormente, edite `menu_bg_HD.png` e rode no Windows:

```bat
py -m pip install pillow
py tools\convert_menu_skin.py
```

Depois copie os `.4444` novos para `memstick/PSP/PLUGINS/TTT_EXPANSION/ui/` e reinicie completamente o PPSSPP. Isso **não requer recompilar o PRX** para alterações no desenho do fundo ou na fonte (desde que mantenha as dimensões esperadas).

**Importante:** a pasta `PSP/TEXTURES/ULUS10537` do sistema de substituição de texturas do PPSSPP é diferente. O PRX lê diretamente as texturas de `PSP/PLUGINS/TTT_EXPANSION/ui/`. Colocar PNGs apenas na pasta `TEXTURES` não será detectado automaticamente por este plugin.

## Índices de CHR preservados

| ID | Goku | Color 1 | Color 2 | Color 3 | Color 4 | SFX reservado |
|---|---|---|---|---|---|---|
| `00` | Início | 14098 (`0x3711`) | 14099 (`0x3712`) | 14100 (`0x3713`) | 14101 (`0x3714`) | 14102 |
| `01` | Mid | 14103 (`0x3716`) | 14104 (`0x3717`) | 14105 (`0x3718`) | 14106 (`0x3719`) | 14107 |
| `02` | Mid SSJ | 14108 (`0x371B`) | 14109 (`0x371C`) | 14110 (`0x371D`) | 14111 (`0x371E`) | 14112 |

SFX novos continuam reservados/não redirecionados. IDs 01/02 ainda compartilham provisoriamente a forma interna 0x47 do experimento: não foram confirmadas animações ou ataques independentes.

## Segurança e diagnóstico

- A lista GU permanece em **128 KB**, não em 512 KB (esta última causou uma tela preta na V0.9.8.8).
- A skin usa texturas externas compactadas de 128 KB + 64 KB carregadas **na abertura do menu** no heap, não incorporadas à memória estática do PRX.
- Um atlas de fontes substitui os milhares de retângulos do antigo texto, reduzindo as primitivas que antes cortavam o painel. O limite de 1200 primitivas por quadro e as sentinelas foram preservados.
- Quando abrir o menu, procure em `diagnostico.txt` a linha `PORTRAITS ... SKIN=0x00000001`. Se `SKIN=0x00000000`, verifique caminhos/nome/tamanho dos arquivos `.4444`.
- Para verificar se a correção de renderização funcionou, confirme 1P, selecione 2P, 3P, 4P, troque Color e volte. Envie `diagnostico.txt` se o menu sumir ou o PPSSPP não iniciar.

**Validação:** testes nativos e inspeção de sintaxe passaram. **Ainda sem compilação real do PRX neste ambiente e sem teste de execução no PPSSPP**; é uma versão beta e exige teste.
