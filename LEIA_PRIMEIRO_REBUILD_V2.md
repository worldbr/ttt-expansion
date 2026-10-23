# TTT EXPANSION DLC — REBUILD 2.0

## Estado de engenharia e limites

- Game: ULUS10537, PPSSPP preferencialmente no PC; **nenhuma garantia de PSP físico ou Android**.
- Header PACKFILE reservado de `0x38000` bytes. Registros têm 16 bytes após os 16 bytes do cabeçalho.
- Original: **0x3711 = 14.097** registros.
- Até **0x37FF = 14.335** registros cabem geometricamente nesse header: **238 adicionais em teoria**.
- **VALIDADO NO PPSSPP PELO USUÁRIO: 0x3734 = 14.132** registros (**35 extras**), EBOOT com opcode correspondente `34 37 1E 34` no ELF em `0x2CBD4`.
- **Não testado:** qualquer contador acima de 0x3734. Haver espaço no header não valida RAM, memória do jogo, scripts ou o carregador.
- Alguns índices antigos podem ter tamanho 0 ou compartilhar offsets. Não significa que são reutilizáveis: apenas análise caso a caso.

## Recursos do plugin

| Personagem | External ID | CHR Color 1..4 (numero 1-based do editor) | SFX apenas reservado |
|---|---|---|---|
| Goku (Início) | 00 | 14098, 14099, 14100, 14101 | 14102 |
| Goku (Meio) | 01 | 14103, 14104, 14105, 14106 | 14107 |
| Goku (Meio) — Super Saiyajin | 02 | 14108, 14109, 14110, 14111 | 14112 |

Somente os 12 CHRs recebem hook. Os SFX continuam os originais; novos mapas ainda não recebem hook. Os 3 personagens compartilham provisoriamente índice de forma interno `0x47` — movesets não são 3 novos conjuntos.

## Caso A — VOCÊ JÁ POSSUI A ISO 0x3734 COM 12 COLORS

1. **Não modifique a ISO.** Faça backup da ISO funcional e dos saves.
2. Extraia o ZIP COMPLETO na raiz do repositório GitHub. Não deixe o código em uma subpasta extra.
3. Use workflow `.github/workflows/compilar-prx.yml`, artifact `TTT-Expansion-PRX-REBUILD-V2000`.
4. Instale a pasta **inteira** `TTT_EXPANSION` do artifact em `memstick/PSP/PLUGINS/TTT_EXPANSION/`. Não basta copiar apenas o `.prx`: o menu usa `ui/*.4444` e `portraits/*.rgba`.
5. Garanta que só um plugin TTT Expansion está ativo. Reinicie completamente o PPSSPP, sem carregar save-state antigo.
6. Teste 1P Goku (Início) Color 1 e batalha. Depois teste 1P Goku Meio e Meio SSJ. Por último 1P a 4P.
7. O menu abre com **L + R + Quadrado**. Setas esquerda/direita nos personagens; abaixo, esquerda/direita mudam Color. L/R mudam jogadores. X confirma. Triângulo muda aba. Quadrado desarma IDs na aba Gokus; Círculo fecha.
8. Na aba **MISSÕES**, cima/baixo escolhem 6 desafios; **X prepara 1P** (retorna a aba Gokus, onde é preciso confirmar X mais uma vez); o jogador entra manualmente no Treinamento. **Quadrado marca como concluída manualmente** e salva em `dlc_progress.bin`. Isto não verifica vitórias.
9. Aba MAPAS é informativa e não injeta mapas. Não habilite mapas experimentais de outro mod sem engenharia reversa independente.

## Caso B — RECONSTRUIR ISO A PARTIR DO PACKFILE ORIGINAL 0x3711

É necessário ter os **12 arquivos CHR próprios**, quatro em `assets/GOKU_INICIO`, quatro em `assets/GOKU_MEIO`, quatro em `assets/GOKU_MEIO_SSJ`, com nomes `C1.PCK1` ... `C4.PCK1`. Sem esses arquivos, não inventamos modelos.

1. Extraia o `PACKFILE.BIN` **COMPLETO** da sua ISO original.
2. Execute no Windows 10 com Python instalado: `py -3 tools\inspecionar_packfile.py --packfile "D:\jogo\PACKFILE.BIN" --json packfile_relatorio.json`.
3. Execute `py -3 tools\dlc_packfile_builder.py --packfile "D:\jogo\PACKFILE.BIN" --output "D:\jogo\PACKFILE_DLC.BIN" --assets "assets"`. O script **recusa** a contagem 0x3734 para impedir que apague seus CHRs já inseridos. Confirme que as 12 entradas têm offsets próprios.
4. Com um **EBOOT descriptografado ELF da variante ULUS10537 que testamos**, execute `py -3 tools\patch_eboot_3734.py --eboot "D:\jogo\EBOOT.ELF" --output "D:\jogo\EBOOT_DLC.ELF"`. O script verifica assinatura e opcode antes de mudar. **NÃO use o EBOOT.OLD ~PSP criptografado de outros mods nem patch em EBOOT arbitrário.**
5. Abra a ISO original no editor de ISO para PSP que permita reconstruir a ISO9660 (por exemplo, UMDGen). Substitua `PACKFILE.BIN` pelos bytes de `PACKFILE_DLC.BIN` e o executável descriptografado pela versão apropriada (respeite o formato de executável utilizado pela ISO). **Recrie a ISO com tamanho de arquivo e diretório ISO9660 atualizados**, NÃO cole bytes no HxD. Mantenha backup separado.
6. Valide contagem/header e os 12 registros com o auditor, instale o PRX e teste o jogo.

## Recursos planejados que NÃO estamos chamando de implementados

- Adicionar mapas de verdade requer índices, estruturas, arquivos de cenários e hook do seletor/loader. Ainda não temos esses endereços validados. A aba de mapas é preparatória.
- Modo História original com cutscenes e batalhas automáticas requer reverse engineering de scripts, transições e loading. O que está implementado aqui é um catálogo de desafios **para jogar manualmente no Treinamento**, com progresso explicitamente manual.
- SFX próprios não são selecionados. Não ultrapassamos os 35 slots comprovados.

## Se o plugin não iniciar ou o menu não carregar

Verifique `memstick/PSP/PLUGINS/TTT_EXPANSION/diagnostico.txt`. Procure `module_start`, `SKIN_BG`, `SKIN_FONT`, `RESOURCE`, `ACTOR INJECT`, `MENU WATCHDOG`. Nunca misture PRX de uma versão com `ui/` de outra. `SKIN=3` é a indicação de fundo e fonte carregados. `SKIN=0` indica que a textura não foi carregada e o plugin mostra fallback mínimo.

## Segurança

Faça backup, teste apenas ULUS10537, desligue cheats paralelos e não use saves importantes. Os testes locais do PC não substituem compilação PSPSDK nem teste em jogo. Se houver crash, desative PRX e volte para ISO intacta.
