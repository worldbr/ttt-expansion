# TTT Expansion V1.0D — Correcao da falha de alocacao do CHR externo

## Causa identificada pelo log da V1.0C

`EXTERNAL STAGE rc=0xFFFFFFE2` e **-30**, valor retornado pelo proprio codigo
quando `malloc(0x141680)` falha. O arquivo foi encontrado e seu tamanho lido;
nao havia ocorrido leitura de conteudo nem validacao de hash.

A V1.0B ja comprovou leitura completa do arquivo `color_1.pck1`, com
`size=read=0x141680` e `fnv1a=0x30448ABA`.

## Correcao experimental da V1.0D

- PSP: troca `malloc`/`free` por `sceKernelAllocPartitionMemory` /
  `sceKernelFreePartitionMemory`, solicitando um bloco da **particao USER**.
  `sceKernelGetBlockHeadAddr` fornece o endereco inicial.
- Registra o identificador do bloco (`block_uid`) e o erro bruto da alocacao
  (`alloc_err`) para diagnosticar realmente falta de memoria livre.
- Libera o bloco anterior na recarga e ao desarmar a ponte.
- Host (testes nativos): continua utilizando malloc/free, pois sao testes fora do PSP.
- Nao injeta CHR na batalha, nao modifica o PACKFILE nem altera IDs ou GPU.
  A batalha continua carregando o recurso da entrada 1194 da ISO.

## Teste

1. Faca backup do PRX da V1.0B/V1.0C.
2. Atualize o projeto no GitHub e execute **Actions > Compilar PRX do menu**.
   Baixe o artifact `TTT-Expansion-PRX-V100D`.
3. Deixe o mesmo `color_1.pck1` em
   `PSP/PLUGINS/TTT_EXPANSION/characters/goku_early/color_1.pck1`.
4. Reinicie completamente o PPSSPP.
5. Abra L+R+Quadrado, selecione 1P > Goku Early > Color 1 e confirme X.
6. Leia `EXTERNAL STAGE` no `diagnostico.txt`.

**Resultado esperado para CHR inalterado:**

```
[TTT V1.0D] EXTERNAL STAGE rc=0x00000000 expected=0x00143000 size=0x00141680 read=0x00141680 first4=0xF1010000 fnv1a=0x30448ABA address=0xXXXXXXXX retained=0x00000001 allocator=0x00000001 alloc_err=0x00000000 block_uid=0xXXXXXXXX (RAM PLUGIN; BATALHA USA ISO 1194)
```

`address` varia. `allocator=1` significa alocacao via particao USER.
`block_uid` deve indicar um identificador PSP valido (nao `0xFFFFFFFF`).

**Se a alocacao falhar** (`rc=0xFFFFFFE2`, `alloc_err` diferente de zero),
nao tente reservar blocos em enderecos fixos: envie o erro bruto para
investigarmos a memoria livre do jogo. O plugin continuara usando a ISO.

**Seguranca:** reservar ~1,31 MB a mais pode reduzir a memoria disponivel para
o jogo. Faca o primeiro teste no menu; so tente a batalha se nao houver
problemas. Se travar, restaure a V1.0B e reinicie o emulador.

**Validacao:** testes nativos passam; compilacao real via PSPSDK e o PPSSPP
precisam ser confirmados pelo usuario.
