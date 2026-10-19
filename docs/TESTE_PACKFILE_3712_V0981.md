# Teste V0.9.8.1 — PACKFILE 0x3711 / 0x3712

## Correção confirmada no código-fonte

Na V0.9.8, `src/resource_hook.c` exigia `count == 0x3711`. Quando o PACKFILE experimental indicava `0x3712`, `ttt_res_hook_install()` retornava -11, e `main.c` não armava `ttt_bridge_install()` nem o hook `ttt_actor_inject_install()`. Resultado: o jogo abria, mas os jogadores não eram substituídos.

Na V0.9.8.1, a validação aceita **somente** `0x3711` e `0x3712`, mantendo os outros requisitos e todos os hooks como antes. Não expandimos as tabelas de personagens nesta versão.

## Como testar

1. Backup da ISO funcional `0x3711`, do EBOOT e da V0.9.8.
2. Use seu PACKFILE **experimental 0x3712 com o novo registro válido**, junto do EBOOT já testado que aceita `0x3712` (sem mexer em mais bytes).
3. Atualize a V0.9.8 com a V0.9.8.1, compile PRX via GitHub Actions (artifact `TTT-Expansion-PRX-V0981`), instale, reinicie PPSSPP totalmente.
4. Abra o menu L+R+Quadrado, escolha Goku Early, Color 1 em 1P, confirme com X, feche. Escolha outro personagem para 1P no seletor original e inicie batalha.
5. Verifique os registros:

```
[TTT V0.9.8.1] RESOURCE CONFIRMAR ... installed=0x00000001 error=0x00000000 ... count=0x00003712
[TTT V0.9.8.1] BRIDGE CONFIRMAR ... installed=0x00000001 error=0x00000000
[TTT V0.9.8.1] ACTOR INJECT STATUS ... installed=0x00000001 ... hits=0x00000001
[TTT V0.9.8.1] RESOURCE MATCH ... id=0x00000000 ... new=0x000004A9
```

Se funcionar, teste 2P/3P/4P como na V0.9.8. Se houver erros ou travamentos, reverta a versão PRX/ISO e envie `RESOURCE CONFIRMAR`, `BRIDGE CONFIRMAR`, `ACTOR INJECT STATUS` e qualquer `... ERRO`.

## Escopo / limites

- **Ainda usa o CHR na entrada 1194 (`0x4A9`), não o novo registro 14098 (`0x3711`)**.
- Compatibilidade com `0x3712` está **corrigida no código e nos testes nativos, não validada no PPSSPP**.
- O PATCH EBOOT do contador precisa estar coerente com o PACKFILE durante o teste; este projeto não o aplica sozinho.
- O novo registro pode ter outras restrições ainda desconhecidas para carregamento efetivo; este teste isola a injeção, não demonstra que 14098 é carregável.
- Esta versão conserva o GPU Guard e o hook Assembly da V0.9.8.
