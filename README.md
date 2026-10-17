# TTT Expansion V1.0C — Mod Loader: preparar CHR externo completo em RAM

**Motivo:** o usuario confirmou `EXTERNAL PROBE rc=0`, `size=read=0x141680`, `first4=0xF1010000`, `fnv1a=0x30448ABA`. A V1.0B conseguiu ler o PCK1 externo, mas descartava cada bloco apos calcular o hash.

## Novidade da V1.0C

- Carrega o **CHR inteiro** em uma unica alocacao privada da memoria do plugin (limite de 4 KiB a 8 MiB).
- Le o arquivo de forma segura em blocos, preserva todos os bytes, calcula FNV-1a e registra `EXTERNAL STAGE`.
- `retained=1` significa que o buffer permanece alocado na RAM do plugin. `address` mostra o endereco do buffer para futuros testes de integracao, **nao** e um endereco para editar manualmente no debugger.
- Libera buffers antigos quando o arquivo e recarregado, se a confirmacao falha ou quando o reset da ponte e acionado. Desalocacao no encerramento normal do worker.
- A ponte ID `00` e o redirecionamento ISO `1194` **nao mudaram**. A batalha **ainda usa o PACKFILE da ISO**. Nenhum ponteiro do jogo e substituido, nenhuma rotina de leitura do jogo e interceptada nesta fase.
- GPU Guard, desenho do menu e controles continuam iguais a V1.0B.

## Como testar

1. Backup da V1.0B, atualize os arquivos do repositorio e compile em GitHub Actions. Artifact `TTT-Expansion-PRX-V100C`.
2. Mantenha o CHR `color_1.pck1` em `PSP/PLUGINS/TTT_EXPANSION/characters/goku_early/`, sem ajustar o tamanho.
3. Reinicie o PPSSPP, abra `L + R + Quadrado`, selecione `1P > Goku Early > Color 1`, confirme X.
4. Procure `EXTERNAL STAGE` em `diagnostico.txt`.
5. A batalha por Videl forma 2 deve continuar usando a entrada ISO 1194, com graficos normais.

**Exemplo esperado (endereco varia por execucao):**

```
[TTT V1.0C] EXTERNAL STAGE rc=0x00000000 expected=0x00143000 size=0x00141680 read=0x00141680 first4=0xF1010000 fnv1a=0x30448ABA address=0xXXXXXXXX retained=0x00000001 (SOMENTE RAM DO PLUGIN; BATALHA USA ISO 1194)
```

Se o conteudo externo nao mudou, FNV-1a deve permanecer `0x30448ABA`. Se aparecer `rc=-30`, houve falta de memoria livre para reter um CHR completo. Isso nao afeta a ponte ID00, que usa ISO 1194. Caso o emulador trave, restaure o backup da V1.0B.

**Limite tecnico:** leitura e armazenamento em memoria nao sao injecao. Precisaremos mapear a interface usada pelo carregador PACKFILE para entregar esse buffer ao jogo sem alterar dados de outros personagens.

## Testes locais

```
gcc -std=gnu11 -Wall -Wextra -Werror -Isrc tests/test_external_stage.c src/external_stage.c src/external_probe.c -o /tmp/ttt_stage_test
/tmp/ttt_stage_test
```

A compilacao PSP depende do seu GitHub Actions, nao foi testada neste ambiente.
