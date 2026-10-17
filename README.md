# TTT Expansion V1.0A - Mod Loader: primeiro teste de leitura externa

**A V0.9.3 ja comprovou o CHR separado do Goku Early (ID 00):** voce trocou o arquivo 1194 pelo CHR SSJ1 e o personagem apareceu SSJ, mantendo ID 00. **Esta V1.0A NAO carrega CHR externo na batalha**. E uma etapa intermediaria necessaria para diagnosticar a leitura externa, preservando o comportamento e o GPU Guard da V0.9.3.

## Arquivos esperados

Na pasta do Memory Stick do PPSSPP (mostrada em Configuracoes > Sistema > Mostrar pasta Memory Stick):

```text
PSP/PLUGINS/TTT_EXPANSION/
  plugin.ini
  ttt_expansion.prx
  portraits/goku_early.rgba
  portraits/goku_mid.rgba
  characters/goku_early/color_1.pck1
```

O arquivo `color_1.pck1` precisa conter bytes de verdade do CHR extraido do jogo (por exemplo, Goku SSJ1 Color 1, item 1433), NAO PNG renomeado. Ele **nao foi distribuido** no ZIP. **Para a V1.0A, o tamanho esperado e 0x143000 (1310016 bytes)**; nao modifique os dados do CHR para atingir o tamanho.

## Teste

1. Faca backup do PRX V0.9.3 e da ISO. Atualize o GitHub com este projeto e execute a action `Compilar PRX do menu`. O artifact sera `TTT-Expansion-PRX-V100A`.
2. Instale o PRX novo e as pastas do artifact no PPSSPP. Copie voce mesmo o CHR PCK1 real para `characters/goku_early/color_1.pck1`.
3. Sem cheats antigos, na tela de selecao abra `L + R + Quadrado`, escolha `1P > Goku Early > Color 1` e confirme X; feche o menu.
4. Confira `diagnostico.txt`. Procure `EXTERNAL PROBE`.

Exemplo esperado (hash e first4 variam):

```text
[TTT V1.0A] EXTERNAL PROBE rc=0x00000000 expected=0x00143000 size=0x00143000 read=0x00143000 first4=... fnv1a=... (SOMENTE LEITURA, SEM CHR EXTERNO EM BATALHA)
```

- `rc=0`: acesso externo completo e leitura de todos os bytes confirmados, MAS o jogo ainda usa a entrada 1194 da ISO.
- `rc=0xFFFFFFFF`: arquivo externo ausente/sem acesso.
- `rc=0xFFFFFFF4`: tamanho diferente do esperado (`-12`).
- `rc=0xFFFFFFFC`: leitura incompleta (`-4`).
- O teste de batalha continua identico a V0.9.3; nao use a aparencia SSJ para concluir que o externo ja foi injetado.

O plugin nunca sobrescreve o arquivo externo, nem altera indice/tabela de PACKFILE alem dos hooks originais validado da V0.9.3. Para a proxima fase (injetar CHR externo na batalha), precisamos estudar o ponto de leitura, alocacao e descompressao do resource ID `0x4A9`, preservando o tamanho e depois suportando tamanho livre.
