# TTT Expansion V1.0.1 — Menu refeito com navegação harmonizada

## Interface

- Três personagens disponíveis, em uma mesma linha (esquerda para direita): **Goku (Inicio)**, **Goku (Meio)**, **Goku (Meio) — Super Saiyajin**.
- **Super Saiyajin** aparece em uma segunda linha, como subtítulo no cartão e na prévia ampliada.
- Segunda linha: três cartões FUTURO meramente visuais; eles não podem ser selecionados.
- Os nomes exibidos pelo PSP usam letras ASCII ("Inicio" sem acento) por causa do atlas da fonte. O estilo é livre para edição.
- Aba PERSONAGENS com painel de prévia, retrato, ID, jogador, status e quatro Colors.

## Controles

| Botão | Ação |
| --- | --- |
| L + R + Quadrado | Abre/fecha o Expansion; aperte Quadrado por último |
| Esquerda / Direita (personagens em foco) | Goku (Inicio) ↔ Goku (Meio) ↔ Goku (Meio) - Super Saiyajin |
| Baixo | Muda o foco para a fileira das quatro Colors |
| Esquerda / Direita (Colors em foco) | C1 ↔ C2 ↔ C3 ↔ C4 |
| Cima | Retorna o foco aos personagens |
| L / R | Troca 1P, 2P, 3P ou 4P, preservando a escolha individual |
| X | Confirma a escolha do jogador atual |
| Quadrado (sozinho) | Desarma a seleção experimental |
| Triângulo | Alterna aba |
| Select | Alterna idioma |
| Círculo | Fecha o menu |

Não há conflito entre horizontal para Goku e horizontal para Color: a borda dourada indica qual fileira está em foco.

## Diagnóstico da versão que realmente carregou

O artefato certo do GitHub Actions é `TTT-Expansion-PRX-V1001`.

1. Faça backup do PRX atual. **Não altere ISO, EBOOT ou PACKFILE.**
2. Extraia o ZIP COMPLETO na raiz do repositório, atualizando arquivos existentes em `src/`, `TTT_EXPANSION/` e `.github/workflows/`.
3. Compile pelo workflow `Compilar PRX menu V1.0.1`.
4. Baixe o artefato e copie **a pasta TTT_EXPANSION inteira**, incluindo `ui/menu_bg.4444`, `ui/font_atlas.4444` e os 3 retratos `.rgba`, para `memstick/PSP/PLUGINS/TTT_EXPANSION/`.
5. Reinicie TOTALMENTE PPSSPP. Não use save-state anterior ao plugin.
6. Abra o menu. A identificação na interface deve ser **V1.0.1**, não V0.9.8.9 e nem V1.0.0.
7. Em `diagnostico.txt`, procure `PORTRAITS ... SKIN=0x00000003`: bit 1 significa fundo carregado, bit 2 significa fonte carregada. `0x3` significa ambos carregados.
8. Se os `.4444` estiverem ausentes ou não corresponderem ao tamanho, a tela mostra `TEXTURAS AUSENTES - VERIFIQUE PASTA UI`, e não o antigo menu. Assim conseguimos identificar a falha de instalação.

### Localização dos arquivos gráficos

- Fundo PSP: `TTT_EXPANSION/ui/menu_bg.4444` (131072 bytes)
- Fonte PSP: `TTT_EXPANSION/ui/font_atlas.4444` (65536 bytes)
- HD editável: `TTT_EXPANSION/ui/menu_bg_HD.png`
- Prévia atualizada: `TTT_EXPANSION/ui/PREVIEW_MENU_REFEITO.png`

O PRX lê os arquivos de `PSP/PLUGINS/TTT_EXPANSION/ui/`. A pasta `PSP/TEXTURES/ULUS10537` do substituidor do PPSSPP é um sistema diferente.

## IDs e Colors (inalterados)

| Forma | ID | C1 | C2 | C3 | C4 | SFX reservado |
| --- | --- | --- | --- | --- | --- | --- |
| Goku (Inicio) | 00 | 14098 | 14099 | 14100 | 14101 | 14102 |
| Goku (Meio) | 01 | 14103 | 14104 | 14105 | 14106 | 14107 |
| Goku (Meio) — Super Saiyajin | 02 | 14108 | 14109 | 14110 | 14111 | 14112 |

Os SFX ainda não são redirecionados. Os IDs 01/02 ainda usam provisoriamente a forma interna 0x47 compartilhada; isso não demonstra animações/movesets independentes.

## Validação

Suíte C nativa e inspeção de sintaxe com headers simulados passaram. **A compilação PRX via PSPSDK e o teste PPSSPP ainda estão pendentes.** Se persistir crash ou menu não carregar, envie `diagnostico.txt` com `V1.0.1`, `SKIN=`, `MENU WATCHDOG` e `GE GUARD`.
