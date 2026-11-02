# TTT Expansion Menu 47 — V0.4.4, recuperação pós-batalha experimental

Jogo ULUS10537, PPSSPP. Atualiza o PRX; **não altera EBOOT V0.4 nem PACKFILE 0x37FC**. Mantenha ambos como estavam na sua ISO de teste.

## Achado no diagnostico(6).txt

A V0.4.3 carrega corretamente no início e aprova os 47 mapeamentos, mas depois de uma batalha o código RAM do resolvedor `0x089C6CA0` deixa de começar com `0x2C8800A1` e aparece `0x682BE5B6`. O segundo comando do resolvedor e os dois primeiros comandos do resolvedor auxiliar permanecem conforme V0.4. A proteção recusava todas as confirmações posteriores com `-42`. O log não mostrava o conteúdo completo dos 38 comandos restantes; por isso **não seria seguro gravar cegamente um patch nessa região**.

## O que mudou

- O verificador compara a **totalidade** dos dois resolvedores V0.4: 24 e 15 palavras MIPS, além da contagem `0x37FC`.
- Somente se **a primeira palavra** de `0x089C6CA0` estiver errada **e todas as outras 38 palavras** continuarem idênticas ao EBOOT V0.4, o plugin restaura exatamente `0x2C8800A1` e invalida caches. Não toca em outros endereços.
- Se qualquer outro comando for diferente, recusa a escrita e informa `CODE_RECOVERY rc=FFFFFFFF module=... word=... actual=... expected=...`.
- Após a nova verificação, pode voltar a confirmar os 47 IDs individualmente e, se necessário, recuperar mapeamentos marcados `FF`. Os três Gokus não são a única opção.
- Registra `ACTOR_SNAPSHOT`, `ACTOR_RESULT` por jogador e `EXPECTED_RESOURCE` com ID, Color, índice esperado e cabeçalho do PACKFILE. Esses dados ajudarão a descobrir por que os placeholders ainda exibem os personagens originais.
- Continua lendo os 47 nomes externos de `TTT_EXPANSION/characters.txt` e usando o mesmo menu sem música.

## Como instalar

1. No GitHub, substitua os arquivos do ZIP de atualização (não deixe vários PRXs carregando simultaneamente).
2. Compile pelo GitHub Actions e baixe **o artefato criado por essa nova execução**. Não use um PRX de build anterior.
3. Substitua `ms0:/PSP/PLUGINS/TTT_EXPANSION/ttt_expansion.prx` pelo novo, mantendo o `characters.txt`, `plugin.ini`, retratos e atlas. Confira que o PRX é carregado pelo PPSSPP (ULUS10537).
4. Reinicie o jogo *completamente*, não apenas via savestate.
5. Confirme que o log contém `[TTT MENU47 V0.4.4] module_start OK`.
6. Faça uma luta com os três primeiros Gokus, finalize, reabra o plugin e teste X novamente. Depois repita usando o slot 08 (ID `0x0B`) e outro slot provisório.
7. Para o jogo mostrar PRONTO no menu original, ainda confirme o personagem portador com X no seletor nativo depois de fechar o plugin.

## Interpretação do próximo diagnóstico

- `CODE_RECOVERY rc=00000001`: a palavra perdida foi restaurada com prova dos outros 38 comandos; teste confirmar normalmente.
- `CODE_RECOVERY rc=FFFFFFFF`: o código da região não está idêntico ao ELF V0.4 (além do primeiro comando); o plugin NÃO tenta corrigi-lo para não sobrescrever o jogo. O campo `module=0` significa o primeiro resolvedor; `module=1`, o segundo. `word=N` indica a posição da primeira divergência.
- `EXPECTED_RESOURCE`: **índice esperado**, não prova que aquele arquivo foi carregado. `ACTOR_RESULT`: último ID presente na RAM ao abrir o menu, não necessariamente o ID após o carregamento do modelo.
- `ACTOR_SNAPSHOT hits` diferente de zero mostra que o shim escreveu um ID em algum ator, mas isso sozinho não comprova que todos os caminhos de carregamento de CHR/SFX foram redirecionados.

## Os 44 personagens provisórios

A estrutura contém 47 IDs e índices `0x3711..0x37FB`, porém os 44 adicionais não possuem CHRs/SFX exclusivos. O PACKFILE V0.4 reutiliza ponteiros do Goku. **IDs novos no menu + ponteiros de Goku não bastam para obrigar todas as rotinas do jogo a carregar Goku**. O relato de personagens originais em batalha pode indicar que algum caminho de inicialização ou de recursos ignora o resolvedor expandido. O log novo ajuda a separar falha do ID de falha do carregamento. Não vamos chamar esses 44 de personagens plenamente funcionais até os recursos aparecerem corretamente numa luta.

## Limites dos testes

Testes locais de código C, navegação e assets passaram. **O PRX PSP não foi compilado localmente e a recuperação ainda não foi testada no PPSSPP.** Caso a região inteira do código tenha sido substituída pelo jogo, a verificação recusará repará-la: isso exigiria investigar o recarregamento de módulos ou outro gancho em vez de sobrescrever dados de memória desconhecidos.
