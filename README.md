# TTT Expansion V0.9.7 — primeiro teste de seleção automática do 1P

**Alvo:** Dragon Ball Z: Tenkaichi Tag Team `ULUS10537`, EBOOT.OLD da nossa investigação.

**Situação:** compilação MIPS completa e teste em PPSSPP **ainda não confirmados**. Esta versão ativa uma alteração EXPERIMENTAL no ID do jogador 1 **antes de uma etapa do carregamento da batalha**; NÃO é ainda um seletor universal. Guarde o PRX V0.9.3/V0.9.6 para reverter.

## O que aprendemos no PPSSPP

O usuário identificou com breakpoint de escrita no ID do 1P:

- PC `0x088576C8`: `sw a0,0(fp)` com `fp=0x08B48AE8`.
- PC seguinte `0x088576CC`: `lw a0,0x18(sp)` — executado como delay slot do nosso `jal`.
- PC `0x088576D0`: `sw a1,8(fp)` — Color armazenada após o ID.
- No EBOOT, `0x088576BC` lê o ID original de `0x20(sp)` e `0x088576C0` lê a Color de `0x1C(sp)`.
- A inicialização do personagem acontece pela rotina `0x08857628`, chamada em pelo menos `0x08857C48` e `0x08857D38`.
- Os endereços de ID/Color 1P–4P seguem o padrão `base + equipe*0x1D0 + slot*0xD8`.

O plugin substitui SOMENTE a instrução `sw a0,0(fp)` por um `jal` a uma pequena rotina Assembly `src/actor_inject.S`. Antes de instalar, confere os dois opcodes originais (`0xAFC40000` e `0x8FA40018`). O restante do jogo segue seu caminho normal.

**Proteção importante:** a rotina Assembly testa o destino `fp == 0x08B48AE8` e o sinalizador de escolha confirmada. Para qualquer outro destino (2P, 3P, 4P, estruturas não relacionadas), reproduz o ID original a partir de `0x20(sp)` e retorna ao jogo. Ela mantém o delay slot sem alterações, e não faz logs nem acessos ao sistema de arquivos.

## Como ativar

Apenas: **1P → Goku Early (ID 00) → Color 1 → X**.

A confirmação instala, nesta ordem:
1. Hook já comprovado de resolução do CHR (`ID 00`, Color 1 -> PACKFILE `0x4A9`, entrada 1194).
2. Ponte do ID 00 nas tabelas A/B do EBOOT, com quinta forma do Goku preservada como fallback.
3. Novo hook da atribuição de ID e Color na inicialização do 1P.

**Nenhum outro personagem/jogador recebe injeção nesta versão.** Confirmar outra escolha para 1P desativa a injeção, sem ampliar suporte a outros IDs. Quadrado no menu do plugin desarma a ponte e restaura a instrução original. A versão mantém GPU Guard e as rotinas de diagnóstico anteriores.

## Teste recomendado, em duas fases

0. Faça BACKUP do PRX anterior e salve o jogo. Teste em PPSSPP desktop, com a mesma ISO modificada (CHR válido na entrada 1194). Desative cheats antigos de expansão.
1. Compile via GitHub Actions (`Compilar PRX do menu`), artifact esperado `TTT-Expansion-PRX-V097`. Instale o PRX e reinicie COMPLETAMENTE o PPSSPP.
2. **Teste A (sem injeção)**: entre em batalha original SEM abrir o plugin. Confirme que os gráficos, personagens e comandos estão normais. O novo hook não deve estar instalado sem confirmação.
3. **Teste B (injeção)**: reinicie o PPSSPP; abra TTT Expansion com **L + R + Quadrado**; selecione **1P / Goku Early / Color 1**, confirme **X**, feche **Círculo**.
4. No menu ORIGINAL escolha um personagem qualquer para 1P (preferencialmente Piccolo ou outro que NÃO seja Goku), complete a seleção e inicie a luta.
5. Verifique se 1P aparece como Goku (CHR da entrada 1194, podendo estar SSJ se foi a cópia escolhida), se P2–P4 continuam originais e se o cenário carrega normalmente.
6. Procure no `diagnostico.txt` os registros `ACTOR INJECT CONFIRMAR`, `ACTOR INJECT STATUS`, `ACTOR CHANGE`, `LOAD TRACE` e `RESOURCE MATCH`.

**Não confunda `installed=1` com sucesso da troca.** Para comprovar a tentativa da intervenção, precisamos de `enabled=1` e `hits>=1`. Para comprovar o resultado, precisamos de `ACTOR RAW` 1P ID `00` e do CHR correto em batalha. Se o ID for sobrescrito por outra etapa, investigaremos as próximas escritas.

### Exemplo do diagnóstico ESPERADO (não comprovado)

```
[TTT V0.9.7] ACTOR INJECT CONFIRMAR installed=0x00000001 enabled=0x00000001 error=0x00000000 hits=0x00000000 ...
[TTT V0.9.7] ACTOR INJECT STATUS installed=0x00000001 enabled=0x00000001 error=0x00000000 hits=0x00000001 original_id=0x00000022 injected_id=0x00000000 ...
[TTT V0.9.7] RESOURCE MATCH ... id=0x00000000 ... new=0x000004A9 ...
[TTT V0.9.7] ACTOR RAW ... P0x00000001_id=0x00000000_color=0x00000000 ...
```

O sistema ainda pode carregar atributos/som do personagem antigo porque o campo de ID é apenas uma das estruturas envolvidas; efeitos ou travamentos NÃO são resultado desejado.

## Limitações e segurança

- IDs do 2P–4P são apenas observados. Não habilitar a troca dos outros jogadores até o teste 1P ser validado.
- Só Goku Early ID 00, Color 1 foi planejado para este hook. Não use outro ID/Color com ele.
- O plugin continua usando a entrada 1194 do PACKFILE, **não** o Mod Loader externo (cuja alocação falhou em V1.0D).
- Se a luta travar, apresentar artefatos ou comportamento inesperado, feche o PPSSPP e restaure o PRX antigo. Não use o botão de reset do plugin durante uma luta em andamento.
- O patch checa os opcodes do EBOOT; se não coincidirem, recusa a instalação e registra erro.

## Arquivos principais

- `src/actor_inject.S`: rotina Assembly MIPS, preserva a instrução e o delay slot.
- `src/actor_inject.c/.h`: instalação/restauração, validação do EBOOT e contadores.
- `src/main.c`: confirmação pelo menu, ativação e diagnóstico.
- `src/battle_bridge.c`: tabelas do ID 00 + quinta forma do Goku.
- `src/resource_hook.c`: redirecionamento CHR 1194.
- `tests/test_actor_inject.c`: testes nativos das salvaguardas do opcode/JAL.
- `docs/BREAKPOINT_PPSSPP.md`: investigação original com debugger.

## Próximo passo (depende dos resultados)

Se `hits>0`, `ACTOR RAW P1=00`, e `RESOURCE MATCH new=0x4A9`, validar visual e golpes. Só depois generalizar mapeamento seguro para 2P–4P e remover a necessidade da quinta forma. Se não houver hits ou ocorrer sobreposição do ID, investigar a ordem dos writes e callsites antes de estender o patch.
