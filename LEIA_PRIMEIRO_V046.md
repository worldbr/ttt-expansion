# TTT Expansion Menu47 V0.4.6 — segundo combate (experimento com confirmação diferida)

**Condição de uso**: somente PPSSPP, jogo ULUS10537, EBOOT V0.4 e PACKFILE contagem `0x37FC`.

## Por que a V0.4.5 falhava

O log `diagnostico(8).txt` mostra a primeira seleção funcionando, `ACTOR_RESULT` com ID correto, `NOVA RODADA`, depois `CODE_RECOVERY rc=0xFFFFFFFD` para o comando 5 de `0x089C6CA0` (`0x682B6E8A` onde se esperava `0x3C0308A7`). `CONFIRMAR FALHOU rc=0xFFFFFFD6` coincide com o X piscando e o som de erro. O jogo reutiliza essa região de código para dados de uma cena diferente. Restaurar palavras isoladas nesse endereço era a estratégia errada.

## Mudanças V0.4.6

1. **NUNCA escreve código do resolvedor enquanto outra cena ocupa esse endereço.** Os reparos V0.4.4/V0.4.5 foram retirados do caminho de confirmação.
2. Se o PRX comprovou no MESMO processo um EBOOT V0.4 intacto e **já detectou uma injeção de personagem na batalha**, a seleção seguinte pode ser **preparada** quando `0x089C6CA0` foi substituído por dados de cena `0x682Axxxx` ou `0x682Axxxx` ou `0x682Bxxxx` e o resolvedor `unchecked` e contagem do PACKFILE seguem íntegros. Essa confirmação é apenas do PLUGIN, não altera o READY original.
3. Durante essa preparação, o PRX apenas **verifica (sem modificar)** que o ID escolhido ainda aponta para o marcador correto `0x80+slot` em `0x08A743E0` e grupo zero em `0x08A69720`. Não aceita um slot cuja tabela foi alterada.
4. Continua a usar a injeção de ID e Color por jogador do EBOOT que já funcionou em sua primeira luta. Os 47 IDs e arquivos de CHR são mantidos.
5. Logs: `RESOLVER_PHASE phase=0` significa código nativo inteiro; `phase=1` significa cena diferente e seleção preparada sem tocar no código; `CONFIRMACAO DIFERIDA` significa X aceito para preparar; `BATTLE_RESOLVER` mostra a instrução do resolvedor quando o jogo realmente instanciou o ator.

## Teste no PPSSPP

Faça backup do PRX e da ISO. Atualize os arquivos no GitHub, compile e substitua SOMENTE o PRX; mantenha EBOOT e PACKFILE atuais. Reinicie o jogo (não use save state antigo). Selecione um ID extra e confirme com X, depois no seletor do jogo selecione um portador e deixe PRONTO, jogue a luta, volte à seleção, abra L+R+Quadrado, selecione OUTRO extra e aperte X. O amarelo CONFIRMADO deve permanecer e não deve fazer som de erro. Feche com O, confirme portador no jogo e inicie a 2ª luta.

### O que precisamos observar

* `RESOLVER_PHASE phase=0x00000001 trusted=0x00000001 actor_hit_seen=0x00000001`
* `CONFIRMACAO DIFERIDA`
* `BATTLE_RESOLVER actor_hits=... checked0=...`
* `ACTOR_RESULT` mostrando o ID novo.

**Limitação importante:** o log até agora não prova que o resolvedor checked é recarregado antes da *segunda* batalha. A V0.4.6 não restaura nem executa o código desse resolvedor; apenas prepara a escolha para que o jogo possa processá-la quando trocar de cena. Se o jogo NÃO recarregar o código, este teste pode falhar ou o combate pode não carregar. Teste com backup e interrompa em caso de crash.

Testes host do código C e verificações estáticas não equivalem a build PSPDEV nem à execução no PPSSPP.


### Correcao de compilacao 2026-11-02
Esta distribuicao inclui um `src/main.c` completo corrigido. A antiga funcao
`native_repair_log` foi removida por inteiro porque nao era chamada e usava
`g_last_repair_mask`, `g_restore_success` e `g_restore_rejected`, variaveis
inexistentes na V0.4.6. Nao apague manualmente linhas isoladas: substitua
`src/main.c` pelo arquivo deste pacote para evitar comandos soltos e os erros
`expected ')' before '&' token` / `conflicting types for log_line`.
A logica de confirmacao pos-batalha nao foi alterada neste ajuste de build.
