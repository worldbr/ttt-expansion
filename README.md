# TTT Expansion V0.8.1 — Menu com visual inspirado na prévia V0.2

**Objetivo:** aproximar o menu PRX (resolução PSP 480 × 272) da prévia HTML V0.2,
sem alterar os testes de personagens já existentes. Parte da V0.8 confirmada pelo
usuário no PPSSPP (controles funcionaram).

## Mudanças visuais
- Moldura azul/ciano, filetes amarelos, título grande e abas de navegação.
- Grade de seis cartões à esquerda, com Goku Early ID `00` e Goku Mid ID `01`,
  e quatro vagas `FUTURO` desabilitadas.
- Painel de detalhes à direita: retrato ampliado, personagem, ID, jogador,
  status de prévia e Colors 1–4 em linha horizontal.
- Abas Mapas e Ajustes mantidas; textos e atalhos enquadrados no rodapé.
- Retratos externos originais preservados: arquivos `.rgba` 128×128 ou 64×64.

## Controles
- **L+R+Quadrado:** abrir/fechar menu.
- **Cima/baixo:** Goku Early / Mid (cartões na mesma coluna).
- **Esquerda/direita:** Color 1–4.
- **L/R isolados:** trocar 1P–4P com seleção preservada por jogador.
- **X:** marcar a seleção em prévia e registrar em `diagnostico.txt`.
- **Triângulo:** trocar aba; **SELECT:** idioma PT/EN/ES; **Círculo:** fechar.

O hook gráfico e o bloqueio de entrada da V0.8 foram mantidos intactos.

**NÃO implementa a seleção jogável de IDs 00/01.** Esses IDs continuam
sem mapeamento válido no TTT original; X não altera dados da batalha.
**NÃO implementa Mod Loader de CHR/SFX.** Os retratos são carregados pela pasta
`PSP/PLUGINS/TTT_EXPANSION/portraits/`, distinta de `PSP/TEXTURES/ULUS10537`.

## Compilação no GitHub Actions
1. Atualize os arquivos com o pacote completo ou aplique apenas o conteúdo
   da atualização na raiz do repositório.
2. Faça commit e execute **Actions → Compilar PRX do menu → Run workflow**.
3. Baixe **TTT-Expansion-PRX-V081** e copie `TTT_EXPANSION` para
   `PSP/PLUGINS/` no Memory Stick do PPSSPP.
4. Reinicie completamente o emulador.

O pacote da atualização substitui apenas `src/gpu_probe.c`, `src/main.c`
e `.github/workflows/compilar-prx.yml`, além deste README.

## Testes
O código de estado/navegação passou os testes C nativos neste ambiente;
**não há PSPSDK instalado aqui**, portanto compile pelo workflow e teste no PPSSPP.
O estilo é uma adaptação da V0.2, não uma cópia pixel a pixel do HTML.
Faça backup do PRX V0.8 antes de testar.
