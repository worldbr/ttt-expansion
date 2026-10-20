# TTT Expansion V0.9.8.7 — 3 Gokus / 4 Colors (experimental)

Base: V0.9.8.5 (que carregou um CHR fisico novo) + sequencia da V0.9.8.4G validada.

## Importante

- **NAO execute `MONTAR_PACKFILE_GOKU_INICIO.bat` de novo:** e uma ferramenta historica para o primeiro CHR, nao para os 12 CHRs que voce ja inseriu.
- **A ISO nao e reconstruida** neste pacote: use sua ISO ja modificada com `PACKFILE` count `0x3734` e EBOOT com o mesmo contador.
- **Nao use este novo plugin como se os 3 personagens estivessem validados**: ID01 e ID02 agora sao testes de combate. O ID00 ja funcionou, mas as suas Colors 2–4 tambem precisam de teste.
- **A/B mappings:** os IDs externos `00`, `01`, `02` compartilham provisoriamente o indice de forma interna `0x47`. Eles podem compartilhar moveset, animacoes, transformacoes ou falhar na batalha. Nao criamos `0x48` e `0x49` sem pesquisa de tabelas internas.
- **SFX nao redirecionados**: reservados indices `0x3715`, `0x371A`, `0x371F`.
- A bridge nao e usada isoladamente: a sequencia de ativacao sempre sera **recurso → bridge → injecao** (3 hooks juntos).
- A fonte do GitHub Actions e `src/` da raiz, junto com `Makefile`. O ZIP completo e para extrair diretamente na **raiz** do repo, nao numa subpasta nova. O artifact esperado e `TTT-Expansion-PRX-V0987`.

## Tabela

| ID | Personagem | Color 1 | Color 2 | Color 3 | Color 4 | SFX |
|---|---|---|---|---|---|---|
| `00` | Goku Inicio | 14098 | 14099 | 14100 | 14101 | 14102 |
| `01` | Goku Mid | 14103 | 14104 | 14105 | 14106 | 14107 |
| `02` | Goku Mid SSJ | 14108 | 14109 | 14110 | 14111 | 14112 |

Na RAM, o plugin retorna **indices zero-based**, portanto os recursos acima tem indice hexadecimal `0x3711` a `0x371E`, com SFX intermediarios saltados.

## Protocolo de teste no PPSSPP (uma mudanca de cada vez)

1. Crie backup do PRX V0.9.8.5 e da ISO funcional. Compile e instale o PRX novo, reiniciando totalmente o PPSSPP.
2. Escolha **1P → Goku Inicio → Color 1 → X**, feche menu, lute. Esta e a regressao do personagem ja comprovado.
3. Reinicie o PPSSPP. Escolha **1P → Goku Inicio → Color 2**. Se funcionar, teste Colors 3 e 4 (reinicie entre tentativas).
4. Reinicie. Teste **Goku Mid → Color 1**, depois **Goku Mid SSJ → Color 1** separadamente. Se travar, pare e envie tela e diagnostico.
5. Por ultimo, teste 1P/2P/3P/4P com personagens e Colors diferentes na mesma luta.

## Diagnostico esperado

`RESOURCE CONFIRMAR ... count=0x00003734 assets12=0x00000FFF` indica que todos os 12 registros CHR possuem offsets plausiveis e distintos entre si. Nao verifica os bytes do arquivo por assinatura; nao e prova isolada da carga de modelos.

`RESOURCE MATCH ... id=0x00000001 offset=0x00000000 ... new=0x00003716` aponta para Goku Mid, Color 1 (entrada 14103).

`RESOURCE MATCH ... id=0x00000002 offset=0x00000000 ... new=0x0000371B` aponta para Goku Mid SSJ, Color 1 (entrada 14108).

`ACTOR INJECT STATUS ... hits=1` junto com o personagem correto **visivel e jogavel** e que valida efetivamente a execucao.

`assets12` faltando bits significa que algum CHR esta com registro invalido, repetido ou alias; essa Color nao sera ativada. Informe o valor para investigarmos.

**Nao use saves importantes para os testes**. Esses patches MIPS sao experimentais e podem provocar crash.
