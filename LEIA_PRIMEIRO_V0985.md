# TTT Expansion V0.9.8.5 — primeiro CHR fisico (Goku Inicio Normal Traje 1)

## Arquivo recebido e inspecionado

- Nome original: `Goku Inicio - Normal (Traje 1)` (enviado sem extensao).
- Embutido no pacote em `EXPANSAO_7_GOKUS/assets/GOKU_EARLY/C1.PCK1`.
- Tamanho exato: **1.241.632 bytes (`0x12F220`)**.
- Assinatura inicial: `00 00 01 F1`, mesma familia de formato do CHR original.
- SHA-256: `beb8dd7d47f3d3dfe2bff8309f6d40cba40b64294b4e0f06e8e6c8c9ea1e35e4`.
- **Nao foi feito teste em PSP/PPSSPP**, portanto compatibilidade da estrutura interna ainda nao comprovada.

## 1. Como montar o novo PACKFILE

1. Extraia o projeto completo para uma pasta com espaco livre de ~1 GB.
2. Extraia o **PACKFILE.BIN COMPLETO** da ISO (nao usar somente o cabecalho de 224 KB).
3. Execute `MONTAR_PACKFILE_GOKU_INICIO.bat "D:\CAMINHO\PACKFILE.BIN"`.
4. O script cria, SEM sobrescrever o original, `PACKFILE_GOKU_INICIO_C1_3734.BIN` na pasta do projeto.
5. Somente a **entrada 14098 (`0x3711`)** recebe o novo CHR fisico, anexado ao fim dos dados.
6. As 34 entradas restantes mantem alias provisoriamente. Os 14.097 registros originais permanecem intactos.
7. O script gera tambem `.json` e `.csv` com a lista dos novos registros.
8. Opcional: `py -3 tools\verificar_chrs_fisicos.py --packfile "PACKFILE_GOKU_INICIO_C1_3734.BIN" --asset "EXPANSAO_7_GOKUS\assets\GOKU_EARLY\C1.PCK1"`.

## 2. ISO e EBOOT

- Reconstrua uma **nova ISO** usando o PACKFILE expandido e atualizando o **tamanho do arquivo no diretorio ISO9660**. Nao cole o novo arquivo apenas com sobrescrita HxD na ISO, pois ele e maior.
- Mantenha o EBOOT com a contagem `0x3734` ja testada. O projeto possui `tools/patch_eboot_contagem.py` se necessario.
- Nao modifique a ISO original ou tente usar um PACKFILE de apenas 224 KB.

## 3. Plugin

- Compile o projeto **completo** desta versao no GitHub Actions (artefato `TTT-Expansion-PRX-V0985`).
- Preservamos a sequencia estavel da V0.9.8.4G: recurso → bridge → injetor.
- Nova validacao admite os aliases antigos; quando a entrada possui offset proprio, so permite contagem `0x3734` + tamanho exato `0x12F220` + offset da RAM maior que o original.
- Reinicie completamente o PPSSPP e confirme **1P → Goku Inicio → Color 1**; depois entre na batalha.
- Em `diagnostico.txt`, confirme: `count=0x3734`, `size_14098=0x12F220`, `off_14098` **diferente** de `off_goku`, `RESOURCE MATCH old=0x593 new=0x3711` e `ACTOR INJECT STATUS hits>0`.

## 4. Importante

Ate este teste real em PPSSPP, **nao esta comprovado que o carregador aceita um CHR com bytes anexados ao PACKFILE**, apenas indices extras como aliases. Se houver travamento, envie o diagnostico e a tela do PPSSPP. O codigo de console/PSP nao foi compilado aqui por ausencia do PSPSDK; os testes nativos e do empacotador foram executados no PC.
