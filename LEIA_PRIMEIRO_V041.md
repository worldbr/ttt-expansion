# TTT Expansion Menu 47 V0.4.1 — segunda batalha / confirmar novamente

Esta atualização altera **somente o código-fonte do plugin**. NÃO modifica EBOOT nem PACKFILE.
Use junto com o seu **EBOOT V0.4 + PACKFILE 0x37FC** já testados.

## Correção

1. Quando o jogo prepara uma batalha, o shim registra quantas vezes escreveu o ID/Color no ator. Ao abrir novamente o menu do plugin após essas escritas, o menu libera as confirmações da rodada anterior (J1–J4) e deixa escolher de novo; ele mantém os últimos personagens e suas Colors como preferência.
2. Antes de reutilizar o hook em uma nova rodada, o plugin verifica se a instrução de escrita do ID ainda está efetivamente patchada. Se o jogo restaurou a instrução original, o PRX a reinstala com validação. Se uma instrução inesperada apareceu, ele recusa a alteração e registra erro.
3. Ao confirmar um jogador, o plugin restaura no hook os outros jogadores já confirmados na mesma preparação, caso tenha precisado reinstalar o hook.
4. O menu indica **OK - CONFIRME NO JOGO** em vez de confundir confirmação interna do plugin com READY da seleção original.
5. O arquivo diagnostico.txt agora registra `CONFIRMAR FALHOU rc=... id=... store_pc=... pack_count=...` quando o hook ou EBOOT recusar uma confirmação.

## Instalação via GitHub

No repositório `worldbr/ttt-expansion`, copie os arquivos do ZIP de atualização mantendo as pastas. Em especial:
- `src/main.c`
- `src/actor_inject.c` e `src/actor_inject.h`
- `src/match_cycle.c` e `src/match_cycle.h` (NOVOS)
- `src/gpu_probe.c`
- `Makefile` (inclui o novo objeto)
- `tests/test_actor_inject.c`, `tests/test_match_cycle.c` e `tests/run_native_tests.sh`

Faça commit e execute `Actions > TTT Expansion Menu 47 - PRX > Run workflow`. Instale **apenas o PRX gerado** se os outros arquivos da V0.4 já estão na pasta correta do PPSSPP.

## Teste no PPSSPP

1. Na seleção original, escolha um personagem original válido como **portador** (por exemplo, Goku base).
2. Abra o plugin (L+R+Quadrado), selecione um extra, aperte X (deve aparecer OK - CONFIRME NO JOGO) e feche com O.
3. **Na seleção ORIGINAL, aperte X para confirmar e mostrar PRONTO**. O PRX não muda diretamente o estado READY do jogo; ele muda o ID/Color no caminho para a batalha.
4. Entre em batalha, termine-a e retorne à seleção.
5. Abra o plugin novamente. Ele deve ter liberado a confirmação anterior e permitir novo personagem/Color. Repita o processo original de marcar PRONTO.

### Limitações

- Detecção de nova rodada usa escrita efetiva do ID no ator (`actor_inject_hits`), não uma variável de cena comprovada. Se nenhuma escrita ocorrer, a liberação automática não será acionada.
- O READY do seletor original ainda depende da confirmação no menu nativo. Automatizá-lo exigiria localizar e validar as rotinas/flags de seleção do jogo; não simulamos um X às cegas enquanto a tela do plugin está aberta.
- O PRX da V0.4.1 ainda não foi compilado no PSPDEV nem executado no PPSSPP neste ambiente. Os testes de lógica do PC passaram.

### Se continuar sem ficar PRONTO

Pressione O para fechar o plugin, selecione um portador no jogo original e aperte X. Se continuar falhando, envie as últimas linhas de `ms0:/PSP/PLUGINS/TTT_EXPANSION/diagnostico.txt`, principalmente `NOVA RODADA` e `CONFIRMAR FALHOU rc=`. Isso permitirá separar falha do hook, falha de EBOOT e falha do READY nativo.
