# TTT Expansion V0.9.8.3 — Corrigida validacao da nova entrada 14098

## Por que a V0.9.8.2 nao trocava o personagem

No diagnostico real da V0.9.8.2:

```text
RESOURCE CONFIRMAR installed=0 error=0xFFFFFFF2
count=0x3712 record_id=0x3711
size_14098=0x143000 size_goku=0x143000
off_14098=0x2258C off_goku=0x2258C
```

O erro `-14` era causado pelo teste de alinhamento `new_offset & 0x7FF`, aplicado diretamente a um campo da tabela **em RAM**. Esse campo esta em unidades internas do carregador, nao necessariamente em bytes. Por isso, o teste rejeitava o registro mesmo sendo identico ao CHR do Goku original.

Na V0.9.8.3, a validacao aceita o alias quando ambos os campos de endereco na RAM sao **iguais e nao zero** e o tamanho coincide. Continuidade das protecoes: PACKFILE `PacK`, `count=0x3712`, ultimo registro com `record_id=0x3711`, tamanho valido, tamanho igual ao Goku original, opcodes dos cinco hooks conferidos. A injeção dos quatro jogadores e o GPU Guard nao foram alterados.

## Requisitos

- **Somente ULUS10537 com EBOOT original compatível previamente corrigido para 0x3712** antes da inicialização.
- ISO de teste com PACKFILE `count=0x3712` e **entrada adicional 14098 apontando para o mesmo CHR Goku Base original**.
- NAO serve para ISO sem expansao (count=0x3711). Guarde backup da V0.9.8 / V0.9.8.1.
- O PRX nao modifica o EBOOT ou a ISO. Ainda nao insere bytes fisicos novos no PACKFILE.

## Teste

1. Substitua o codigo no GitHub e execute **Actions -> Compilar PRX do menu**.
2. Baixe `TTT-Expansion-PRX-V0983`, instale e reinicie completamente o PPSSPP.
3. Abra com L+R+Quadrado, selecione **Goku Early ID00 / 1P / Color1** e confirme com X.
4. Feche o menu com Circulo. No seletor original escolha outro personagem para 1P e entre em batalha.
5. Procure no diagnostico `RESOURCE CONFIRMAR`, `ACTOR INJECT STATUS`, `RESOURCE MATCH`, `LOAD TRACE`, `ACTOR RAW`.

### Valores esperados (NAO confirmados ainda no PPSSPP)

```text
[TTT V0.9.8.3] RESOURCE CONFIRMAR installed=0x00000001 error=0x00000000 ... count=0x00003712 record_id=0x00003711 off_14098=0x0002258C off_goku=0x0002258C
[TTT V0.9.8.3] ACTOR INJECT STATUS installed=0x00000001 enabled=0x00000001 ... hits=0x00000001
[TTT V0.9.8.3] RESOURCE MATCH ... id=0x00000000 offset=0x00000000 old=0x00000593 new=0x00003711
```

Se o jogo abrir, entrar em batalha com Goku e mostrar `new=0x3711`, havera evidencia de uso do novo **indice adicional**. Isso ainda NAO confirma inserir um novo CHR fisico: o registro novo e um alias do CHR existente. Depois podemos testar um CHR visualmente distinto e, somente apos confirmar a estabilidade, uma entrada com dados acrescentados.

**Seguranca:** faca backup do PRX e ISO. Se ocorrer travamento, reinicie o emulador e restaure o backup. Nao desarme a ponte durante batalha.

## Arquivos modificados em relacao a V0.9.8.2

- `src/resource_hook.c`: remove teste de alinhamento indevido do endereco codificado em RAM; mantem igualdade e nao-zero.
- `src/resource_hook.h`: comentario de especificacao.
- `tests/test_resource.c`: casos baseados no log real com `0x2258C`.
- `src/main.c`: apenas identificadores de versao no diagnostico.
- `.github/workflows/compilar-prx.yml`: artifact `TTT-Expansion-PRX-V0983`.
- `README.md`, `LEIA_PRIMEIRO.txt`, `docs/TESTE_NOVA_ENTRADA_14098_V0983.md`: instrucoes.

Testes locais nao substituem a compilacao do PRX pelo PSPSDK nem seu teste de carregamento no PPSSPP.
