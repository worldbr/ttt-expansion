# TTT Expansion V0.9.8.4G — Teste de compatibilidade estrito (base V0.9.8.3)

**Por que:** V0.9.8.4F também crashou com o PACKFILE `0x3712` em modo 3P (somente bridge).
Isso significa que o teste de bridge isolada não permite responsabilizar as 35 entradas adicionais.

## O que o PRX faz

- Usa o **mesmo fluxo da V0.9.8.3 comprovada**, em `src/main.c`:
  1. `ttt_res_hook_install()`
  2. `ttt_bridge_install()`
  3. `ttt_actor_inject_install()`
  4. `ttt_actor_inject_enable_slot()`
- A única alteração funcional além dessa base está em `src/resource_hook.c`:
  aceita `count==0x3712` **OU** `count==0x3734` com a entrada **14098** válida.
- `ttt_res_validate_alias` continua rígida: registro 14098 deve apontar para os mesmos dados do Goku original; este teste **não** insere CHR novo.
- As 35 entradas extras ficam na ISO. Os outros seis Gokus ainda **não** são jogáveis nesta versão de isolamento.
- As mensagens antigas do log são majoritariamente `V0.9.8.3`, propositalmente, pois preservamos `main.c` quase integralmente; a primeira mensagem `module_start` identifica `V0.9.8.4G`.

## TESTE A (obrigatório primeiro)

1. Faça backup da ISO e PRX.
2. Use a ISO **anterior com `0x3712` + EBOOT que espera `0x3712`**, já comprovada.
3. Compile o projeto **completo** pelo GitHub Actions. Instale o PRX novo, desligue o PPSSPP e inicie de novo. Não misture arquivos `.o` de builds anteriores.
4. Selecione no nosso menu **1P / Goku Early / Color 1** e pressione X. Há seleção normal, **NÃO existem modos de diagnóstico por jogador** aqui.
5. Feche o menu e inicie a batalha. No diagnóstico, procure `RESOURCE MATCH old=0x593 new=0x3711` e `ACTOR INJECT hits>=1`.

## TESTE B (somente se A funcionar)

1. Reinicie completamente o PPSSPP.
2. Use a ISO experimental com PACKFILE **`0x3734` + EBOOT que espera `0x3734`**, cabeçalho correto.
3. Use **exatamente o mesmo PRX** da V0.9.8.4G.
4. Repita Goku Early / 1P / Color 1; somente depois teste os outros jogadores.

### Interpretação
- A funciona + B funciona: expandir a quantidade para 35 registros parece compatível com a cadeia normal; próximo passo é inserir um CHR fisicamente novo.
- A funciona + B trava: a diferença está ligada ao carregamento/cabeçalho/EBOOT `0x3734`, não à bridge isolada.
- A trava: regressão independente do tamanho do PACKFILE, investigar PC via `dist/debug/ttt_expansion.elf` e diagnóstico; comparar com V0.9.8.3 comprovada.

**Não confirme Goku usando 3P esperando que ele seja teste isolado**; aqui 1P-4P são jogadores de verdade.

Não testado em hardware/PPSSPP neste ambiente; os testes locais são de compilação nativa e validação de fonte.
