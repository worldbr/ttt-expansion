# TTT Expansion V0.9.8.4F — isolamento específico da bridge

O diagnóstico enviado mostra que 1P (sem hooks) e 2P (somente recurso) funcionaram,
enquanto 3P (recurso + bridge) travou imediatamente após o registro `DIAG_APOS_GPU_MENU`.
A bridge é binariamente a mesma implementação da V0.9.8.3; não é possível atribuir
culpa exclusiva à bridge sem testar sua execução isolada.

## Controles de diagnóstico: personagem Goku Early, Color 1

- 1P = nenhum hook de personagem (controle)
- 2P = somente hook de recursos (controle)
- 3P = SOMENTE bridge, **sem hook de recursos** e sem injeção de personagem
- 4P = recurso + bridge, sem injeção de personagem (reprodução do crash)

**Nunca tente entrar em batalha com os modos parciais.** Use somente o menu de
seleção do plugin. **Reinicie por completo o PPSSPP entre os testes**, porque
um hook instalado não é automaticamente removido ao trocar o jogador.

## Testes prioritários

1. Com ISO expandida `0x3734` e EBOOT correspondente, reinicie o PPSSPP,
   escolha diretamente **3P** e confirme Goku Early Color 1 uma vez. Registre se trava.
2. Reinicie, escolha diretamente **4P** e confirme Goku Early Color 1. Registre se trava.
3. Se possível, use uma cópia comprovada do PACKFILE com `0x3712` e EBOOT
   `0x3712` e repita o teste 3P depois de reiniciar. Este mesmo plugin aceita
   ambos os contadores, de forma que comparamos a ISO mantendo o código constante.

Os 35 registros não são removidos pelo plugin: a versão F não modifica PACKFILE ou
EBOOT. Apenas identifica se o salto inválido é disparado pela bridge isoladamente
ou pela combinação bridge + hook de recursos em `0x3734`.

Os campos que precisamos no diagnóstico são `DIAG MODO`, `ETAPA`, `RESOURCE
CONFIRMAR`, `BRIDGE CONFIRMAR` e os `DIAG_POS_X_FRAME`. Guarde captura de `PC`,
`Destination`, `RA` em caso de crash.

Esta versão **não instala a injeção dos jogadores** em nenhum modo, então o Goku
não deverá substituir personagens originais. A função dela é diagnosticar,
não substituir a V0.9.8.3 funcional.
