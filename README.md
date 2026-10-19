# TTT Expansion V0.9.8 — Injeção independente 1P–4P (EXPERIMENTAL)

**Jogo:** Dragon Ball Z: Tenkaichi Tag Team — `ULUS10537`, mesmo EBOOT.OLD e PACKFILE da pesquisa.

## O que já foi confirmado pelo usuário (V0.9.7)

O usuário configurou Goku Early ID `00`, Color 1, para 1P; escolheu um personagem diferente no seletor original; entrou em batalha com Goku, jogou, usou golpes e finalizou a luta normalmente. O diagnóstico confirmou `ACTOR INJECT STATUS hits=1`, ID `00` no 1P e `RESOURCE MATCH ... new=0x4A9` (arquivo 1194 do PACKFILE).

A V0.9.8 **não foi compilada pelo PSPSDK nem executada no PPSSPP neste momento**. A ampliação 2P/3P/4P é experimental. Faça backup da V0.9.7.

## Novidade V0.9.8

O hook em `0x088576C8` foi estendido para reconhecer a estrutura de cada jogador:

| Jogador | ID na RAM | Color na RAM | Máscara |
| --- | --- | --- | --- |
| 1P | `0x08B48AE8` | `0x08B48AF0` | `0x1` |
| 2P | `0x08B48BC0` | `0x08B48BC8` | `0x2` |
| 3P | `0x08B48CB8` | `0x08B48CC0` | `0x4` |
| 4P | `0x08B48D90` | `0x08B48D98` | `0x8` |

Cada jogador é habilitado e desabilitado de forma independente. Uma confirmação de Goku Mid ou uma Color diferente desativa **somente aquele slot**, preservando os outros. **Por enquanto, apenas Goku Early ID 00 / Color 1 é injetável; outros nomes no menu são prévias**. Os CHRs continuam vindo do PACKFILE modificado da ISO, entrada 1194/índice `0x4A9`; não há Mod Loader externo.

A rotina Assembly (`src/actor_inject.S`) compara o registrador `fp` aos quatro endereços exatos, verifica o bit daquele jogador e só substitui quando o ID original é válido (intervalo `0x01..0xA0`). Se o slot estiver vazio (`0`) ou inválido, o código original continua. O hook salva/restaura `$t0`, `$t1`, `$t2` e o stack pointer; preserva a instrução no *delay slot* e altera `$a1` apenas para o slot habilitado. A validação dos dois opcodes originais do EBOOT ocorre antes de instalar o patch.

## Procedimento recomendado — testar uma etapa por vez

1. Faça backup do PRX V0.9.7 e, de preferência, do save. Não use cheats de outros patches de expansão. Mantenha a mesma ISO modificada com um CHR válido na entrada 1194.
2. Atualize o repositório com o projeto completo V0.9.8 ou a atualização e compile por **Actions → Compilar PRX do menu**. Baixe `TTT-Expansion-PRX-V098`.
3. Teste o **1P sozinho**, como na V0.9.7, confirmando Goku Early Color 1, escolhendo outro personagem no seletor original e entrando em batalha. O resultado deve continuar normal.
4. Reinicie o PPSSPP e teste **somente 2P**: abra o Expansion com L+R+Quadrado, mude para 2P, escolha Goku Early / Color 1, confirme X e feche com Círculo. No menu original selecione uma **batalha com 2P ativo**, idealmente 2x2, e escolha um personagem diferente para 2P. Observe o resultado.
5. Apenas se 2P funcionar sem erros, repita com **3P**, depois **4P**, individualmente, sempre reiniciando o emulador para separar os testes. Por fim, poderá testar múltiplos slots ao mesmo tempo.
6. Envie `ACTOR INJECT STATUS`, `ACTOR INJECT SLOT player=...`, `RESOURCE MATCH`, `LOAD TRACE` e `ACTOR RAW` após entrar em batalha.

**Importante:** dependendo do modo de batalha, o 2P/4P pode não estar ocupado. O hook não deve criar um personagem em slot vazio; `hits=0` num slot não utilizado é esperado. Uma batalha 2x2 é melhor para avaliar os quatro jogadores.

## Diagnóstico esperado (exemplo, não é resultado testado)

```
[TTT V0.9.8] ACTOR INJECT STATUS installed=0x00000001 enabled=0x00000001 mask=0x00000002 error=0x00000000 hits=0x00000001 player=0x00000002 ...
[TTT V0.9.8] ACTOR INJECT SLOT player=0x00000002 enabled=0x00000001 hits=0x00000001 original_id=0x00000019 ... want_id=0x00000000 want_color=0x00000000
[TTT V0.9.8] RESOURCE MATCH count=0x00000001 id=0x00000000 offset=0x00000000 old=0x00000593 new=0x000004A9 ...
```

O 2P deve exibir ID 00 em `ACTOR RAW`. O valor `original_id=0x19` é só exemplo, dependerá da seleção feita. `mask=0x2` significa apenas 2P, `mask=0xF` significa todos os jogadores.

## Limitações

- Ainda não há cadastro universal para todos os personagens. Goku Mid ID 01 e outras Colors não devem ser injetados nesta versão.
- Um Goku SSJ na entrada 1194 produzirá essa aparência mesmo com o ID Goku Early 00.
- A criação de personagens completamente independentes (golpes, SFX, atributos, transformações) ainda exige pesquisar novas estruturas do jogo.
- Repetir o ID 00 em vários jogadores ainda não foi testado; recomendamos começar com um só.
- Não tente desarmar a ponte durante uma batalha; reinicie o PPSSPP e restaure a V0.9.7 se houver problemas.
- A quinta forma do Goku segue como fallback da ponte (mantida de versões anteriores), mas você já não precisa escolhê-la manualmente para testar a injeção.
- O programa não foi executado em console PSP real e pode depender do comportamento do PPSSPP.

## Arquivos principais

- `src/actor_inject.S`: interceptação Assembly MIPS com seleção 1P–4P e proteção de slots vazios.
- `src/actor_inject.c/.h`: configuração e máscaras por jogador, diagnóstico e salvaguardas.
- `src/main.c`: integração com menu e log `ACTOR INJECT SLOT`.
- `src/slot_plan.c`: preferências independentes de cada jogador.
- `src/battle_bridge.c`: tabela A/B do ID 00.
- `src/resource_hook.c`: CHR da entrada 1194.
- `tests/test_actor_inject.c`: teste nativo do mapeamento exato 1P–4P.

## Compilação / validação realizada

- `bash tests/run_native_tests.sh`: passou.
- `clang -target mipsel-unknown-elf -c src/actor_inject.S`: montou sem erros (objeto MIPS32).
- Verificação PSP SDK completa / execução em PPSSPP: **pendentes**.
