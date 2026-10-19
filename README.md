# TTT Expansion V0.9.8.4 — 7 Gokus / 35 novas entradas (experimental)

Extensão da V0.9.8.3 **comprovada pelo usuário** em `0x3712`, agora preparando uma contagem de `0x3734` para 7 formas × (4 Colors + SFX).

**LEIA PRIMEIRO:** [`EXPANSAO_7_GOKUS/LEIA_PRIMEIRO_PTBR.md`](EXPANSAO_7_GOKUS/LEIA_PRIMEIRO_PTBR.md).

Requisitos para o experimento: EBOOT.OLD ELF ULUS10537 com contador `0x3734`, PACKFILE original/experimental atualizado com as 35 entradas válidas e PRX compilado V0.9.8.4. O PRX só injeta em batalha `Goku Early ID00 Color1`; as outras formas são provisórias até conseguirmos os assets PCK1 e os mapeamentos completos do EBOOT.

O HxD-ready `EXPANSAO_7_GOKUS/CABECALHO_35_SLOTS_0x3734.bin` cria 35 ALIASES — teste de tabela, **não** modelos reais. `tools/expandir_packfile_7_gokus.py` cria um novo PACKFILE.BIN, adicionando os dados reais fornecidos em uma pasta; não edita ISO nem seus metadados ISO9660.

GitHub Actions: `.github/workflows/compilar-prx.yml` -> artifacts `TTT-Expansion-PRX-V0984` e `TTT-Expansion-Build-Log-V0984`.

Testes sem PSPSDK: `bash tests/run_native_tests.sh`. Os quatro hooks de personagem antigos, guard GPU e desenho de menu foram preservados, salvo expansao da lista e contagem.
