# TTT Expansion Menu47 V0.4.7 - Terceira luta e seguintes

**Problema comprovado em diagnostico(8) (V0.4.6):** a segunda partida injetou IDs 0x16, 0x01 e 0x02 com hits=1 por jogador; na selecao seguinte apareceu `RESOLVER_PHASE phase=0xFFFFFFFD` e a confirmacao voltou a retornar `-42`. A primeira e a segunda instrucoes do resolvedor `unchecked` estavam intactas, mas outra word mudou na transicao de cena.

**Correcao:** ao reconhecer a mesma sessao que ja exibiu EBOOT V0.4 intocado e recebeu ao menos uma injeção de ator, o plugin aceita *preparar* novas escolhas com a regiao de codigo usada por outra cena, mesmo se outras instrucoes de `unchecked` mudaram. Para aceitar, confere `PACKFILE count=0x37FC`, assinatura 0x682A/0x682B na word0 de `checked`, word1 de `checked`, words0/1 de `unchecked`, e **valida o mapeamento exato do ID escolhido**. Nenhuma instrucao das cenas e reescrita.

**Instrucoes:** Atualize os arquivos do repositório com o pacote de GitHub, compile no GitHub Actions e substitua apenas `ttt_expansion.prx` no PPSSPP. Nao altere novamente o EBOOT V0.4 ou PACKFILE. Reinicie o emulador sem save state anterior ao teste. Confirme personagens diferentes nas partidas 1, 2, 3 e 4.

**Logs esperados:** `RESOLVER_PHASE phase=0x00000001` + `CONFIRMACAO DIFERIDA` nas rodadas seguintes, e `ACTOR_RESULT ... inject_hits=0x00000001` apos cada nova batalha. Em caso de falha, enviar diagnostico integral (agora indica `bad_module`, `bad_word`, `bad_actual`, `bad_expected`). Os testes locais cobrem codigo puro; comportamento PPSSPP ainda deve ser verificado.
