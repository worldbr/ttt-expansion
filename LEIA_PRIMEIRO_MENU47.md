# TTT Expansion — Menu 47 V0.1.2 (correções de interface + efeitos)

Esta versão corrige o menu que você mostrou em sua captura do PPSSPP. **Fonte PSP + prévia HTML, ainda não PRX compilado.** Use o GitHub Actions incluído para compilar.

## O que foi corrigido

- 47 quadros visuais, 8 colunas por 6 linhas; 47 retratos, sem números dentro dos quadros.
- 3 Gokus originais da expansão exibidos com suas imagens; removida faixa preta que ocultava 6 px inferiores de cada retrato.
- Slots 04–47 com 44 retratos genéricos provisórios diferentes. Esses NÃO são personagens habilitados na batalha.
- Rodapé em **uma única linha**: `SETAS MOVER  L/R JOG  TRI COR  X OK  O VOLTAR`.
- **Sem música** e sem opção SELECT; efeitos sonoros apenas nos comandos.
- Áudio PCM original curto nas ações: setas, TRIÂNGULO, L/R, X e voltar. O plugin tenta reservar canal PSP somente ao emitir um efeito, para não ocupar o áudio permanentemente. O volume de saída foi aumentado. Se os canais estiverem ocupados pelo jogo, o som pode não sair; verifique o campo `audio_rc` no diagnóstico `TTT_EXPANSION/diagnostico.txt`.
- Quatro jogadores e Colors independentes, como antes.

## Controles

- Abrir/fechar: segurar L+R e pressionar QUADRADO.
- SETAS: mover seleção.
- TRIÂNGULO: alternar grade / Colors.
- L ou R: escolher J1 a J4.
- X: confirmar somente Goku Início, Meio ou Meio SSJ.
- BOLA: fechar.
- QUADRADO: limpar seleções dos quatro jogadores.
- SELECT: sem função neste menu (música foi removida).

## Instalação após a compilação no GitHub Actions

1. Faça backup da ISO e da pasta `PSP/PLUGINS/TTT_EXPANSION`.
2. Envie o código-fonte a `worldbr/ttt-expansion` preservando a estrutura de pastas.
3. Execute o workflow `TTT Expansion Menu 47 - PRX` no Actions.
4. Baixe o artefato e copie a pasta `TTT_EXPANSION/` inteira, incluindo `portraits/placeholder_atlas.rgba` (65536 bytes) e `ttt_expansion.prx`.
5. Teste primeiro a exibição e os sons; depois a escolha dos 3 Gokus, sempre com sua ISO já validada com EBOOT V0.3 e PACKFILE contagem `0x3734`.
6. Se o áudio falhar, veja `audio_rc` no arquivo `diagnostico.txt`: erro de reserva de canal indica que a engine pode estar usando todos os canais PSP.

## Arquivos adicionais

- `PREVIA_INTERATIVA_MENU47.html`: prévia navegável no PC, sem BGM.
- `IMAGEM_PREVIA_MENU47_V012.png`: captura da prévia.
- `TTT_EXPANSION/portraits/placeholders/slot_04.png` ... `slot_47.png`: retratos provisórios editáveis.
- `TTT_EXPANSION/portraits/placeholder_atlas.rgba`: atlas de 128×128, 16×16 por retrato, carregado no menu PSP em apenas 64 KiB.
- `PLANO_47_SLOTS.csv/.json`: índices calculados para 47 grupos de cinco recursos.
- `tools/construir_packfile_47.py`: construtor que só trabalha quando existem os 220 CHR/SFX necessários. Não cria personagens falsos.
- `PREPARACAO_NAO_INSTALAR/EBOOT.V03_CONTADOR_37FC_SOMENTE_EXPERIMENTAL.ELF`: **não instalar sem PACKFILE `0x37FC` completo e sem adaptar novos IDs nativos**.

## Limitações

O PPSSPP e a compilação PRX desta nova revisão ainda não foram testados. Testes C locais passaram. O jogo tem 47 posições VISUAIS, mas apenas três correspondências de batalha implantadas. Sem o PACKFILE completo (~960 MB), 44 conjuntos CHR/SFX e pesquisa dos novos battle IDs, não é possível fornecer um PACKFILE 47 jogável. Não instale o EBOOT preparatório na sua ISO atual.
