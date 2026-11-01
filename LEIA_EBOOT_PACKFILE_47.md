# PACKFILE / EBOOT — 47 slots

## O que podemos comprovar

- Capacidade teórica do índice: `0x37FF` entradas; 47 grupos novos x 5 = 235 registros além de `0x3711`, produzindo a contagem `0x37FC` (sobram 3 registros).
- ISO com `0x3734` já possui 35 registros adicionais; seriam mais 200 registros no índice.
- Os slots 01, 02 e 03 usam os CHRs experimentais e IDs 00, 01, 02; os outros 44 NÃO têm recursos fornecidos.
- Para 44 formas/personagens novos com quatro Colors e SFX separados faltam até 220 arquivos reais.

## Não instalar o contador 0x37FC sozinho!

Em `PREPARACAO_NAO_INSTALAR/` há um EBOOT V0.3 com **somente** a contagem do PACKFILE alterada de `0x3734` para `0x37FC`, de mesmo tamanho. É uma preparação estrutural, **NÃO é um EBOOT para rodar com o PACKFILE atual**. Sem os registros correspondentes pode causar leitura inválida, travamento e corrupção de dados em memória. O EBOOT V0.3 só resolve os IDs de combate 00,01,02; os 44 demais não foram implementados.

## Como construir o futuro PACKFILE 47

Precisa do PACKFILE completo da ISO (o arquivo de ~960 MB; não apenas cabeçalho) e pastas `SLOT_04` até `SLOT_47`, cada uma contendo `C1.PCK1`, `C2.PCK1`, `C3.PCK1`, `C4.PCK1` e `SFX.PCK1`. Com tudo isso, rode:

```bat
py tools\construir_packfile_47.py --packfile PACKFILE.BIN --output PACKFILE_47.BIN --assets assets47
```

O script recusa recursos ausentes e PACKFILE inválido, conserva o original e cria um manifesto; só então usar o EBOOT do contador 0x37FC em uma ISO de TESTE. Isso ainda não tornaria os 44 personagens jogáveis: precisamos atribuir IDs e demais estruturas no EBOOT.

## EBOOT V0.3 de uso atual

O `EBOOT.V03_RESOLVER_TESTADO_3734.ELF` é apenas uma cópia do patch de IDs 00–02 que você já testou. Não possui 47 IDs nativos.
