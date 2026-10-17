# TTT Expansion V1.0B — Mod Loader: teste de leitura com CHR de tamanho variável

**Motivo da atualização:** o usuário forneceu `EXTERNAL PROBE rc=0xFFFFFFF4 expected=0x00143000 size=0x00141680 read=0`. Na V1.0A, `-12` era gerado pela função de validação porque `size != expected`. O plugin **já tinha encontrado o arquivo**; a leitura era interrompida antes do primeiro byte.

## O que mudou

- O valor `expected=0x143000` agora é **somente uma referência**, não uma exigência. O arquivo `0x141680` informado pelo usuário é aceito, desde que realmente possa ser lido.
- O tamanho real é medido por `sceIoLseek`, limitado a **4 KiB até 32 MiB** por segurança. Arquivos vazios, minúsculos ou grandes demais continuam bloqueados.
- O plugin lê os bytes em blocos de 4096, registra `read`, os primeiros quatro bytes (`first4`) e o hash FNV-1a (`fnv1a`).
- `size_match=0` informa que o tamanho difere do original. Não é erro.
- A ponte ID 00, a entrada ISO 1194, o menu e o GPU Guard permanecem iguais aos da versão anterior.

**IMPORTANTE: esta versão NÃO substitui o CHR na batalha pelo arquivo externo.** O teste é apenas de leitura. O Goku Early continua usando a entrada 1194 da ISO, como na V0.9.3.

## Como instalar e testar

1. Faça backup do PRX V1.0A e mantenha a ISO modificada que já funciona.
2. Atualize os arquivos do repositório usando o ZIP e execute a ação `Compilar PRX do menu` no GitHub.
3. Baixe o artifact `TTT-Expansion-PRX-V100B` e coloque os arquivos no Memory Stick do PPSSPP.
4. Preserve seu CHR extraído **sem editar o tamanho nem adicionar bytes** neste caminho:

```text
PSP/PLUGINS/TTT_EXPANSION/
  characters/
    goku_early/
      color_1.pck1
```

5. Reinicie completamente o PPSSPP. Abra o menu `L + R + Quadrado`, selecione **1P > Goku Early > Color 1** e confirme com X.
6. Procure `EXTERNAL PROBE` em `PSP/PLUGINS/TTT_EXPANSION/diagnostico.txt`.

**Exemplo esperado com o arquivo reportado (o hash e os primeiros bytes não são previamente conhecidos):**

```text
[TTT V1.0B] EXTERNAL PROBE rc=0x00000000 expected=0x00143000 size=0x00141680 read=0x00141680 first4=... fnv1a=... size_match=0x00000000 (SOMENTE LEITURA, SEM CHR EXTERNO EM BATALHA)
```

`rc=0` e `read=size` comprovam que o PRX conseguiu ler o arquivo externo completamente. Isso **ainda não prova a validade integral do formato PCK1**, nem a injeção dos dados no jogo.

### Códigos de erro

- `rc=0xFFFFFFFF` (`-1`): `sceIoOpen` não abriu o arquivo; confira pasta, nome e permissões.
- `rc=0xFFFFFFFE` (`-2`): falha em obter tamanho utilizável.
- `rc=0xFFFFFFF5` (`-11`): tamanho fora do intervalo permitido (4 KiB–32 MiB).
- `rc=0xFFFFFFFD` (`-3`): não foi possível voltar ao início.
- `rc=0xFFFFFFFC` (`-4`): leitura não retornou a quantidade solicitada.

O teste é READ-ONLY: nada é escrito no CHR, e não é necessário reconstruir a ISO.
