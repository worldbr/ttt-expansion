# TTT Expansion PRX V0.9 — ID 00 em batalha (teste experimental)

Base: V0.8.1 aprovada no PPSSPP, mantendo interface, 1P–4P, bloqueio de comandos, retratos e atalho L+R+Quadrado.

## O que esta V0.9 realmente faz

Ao pressionar **X no Goku Early do 1P**, tenta instalar um **redirecionamento temporário de IDs** no EBOOT.OLD original ULUS10537 (sem editar a ISO):

| Endereço RAM | Valor anterior esperado | Valor novo | Efeito |
| --- | --- | --- | --- |
| 08A69720 | FF | 00 | ID externo 00 carrega os recursos originais do Goku Base (índice 0) |
| 08A743E0 | FF | 47 | ID externo 00 usa a posição de forma interna 71 |
| 08A740FC | 000000FF | 00000047 | Segunda forma do grupo da Videl aponta para posição 71 |
| 08A74100 | 00000000 | 000000FF | Novo terminador da lista de formas |
| 08A0662C | 2CD10046 | 2CD10048 | Expande um limite local de formas de 70 para 72 |

**O ID 00 pode não ser aceito por todas as rotinas do motor**: é reservado/inativo no TTT original. Nosso objetivo é verificar se a batalha conserva o ID 00 e carrega Goku Base como recurso temporário. O teste de A0 anterior não prova que 00 será válido. **Não adiciona ainda CHR/SFX próprios**, nem implementa carregamento externo de PCK1.

**Goku Mid ID 01 e jogadores 2P–4P continuam em prévia**, pois ainda não há novos recursos/dados independentes para eles.

## Teste no PPSSPP

1. **Faça backup** do PRX V0.8.1 e use uma ISO com o EBOOT.OLD original e cheats de expansão anteriores DESATIVADOS (principalmente Videl, A0, Color). O plugin **recusa** o patch se os valores esperados forem diferentes.
2. Instale o V0.9 compilado pelo GitHub Actions e reinicie o PPSSPP totalmente, sem savestate.
3. Abra a seleção de personagens. Pressione L+R+Quadrado para abrir o TTT Expansion.
4. Escolha 1P, Goku Early ID 00. Pressione **X uma vez**.
5. No `diagnostico.txt`, verifique `BRIDGE CONFIRMAR installed=0x00000001 ... A00=0x00000000 B00=0x00000047`.
6. Feche com Círculo. **No menu ORIGINAL**, selecione **Videl** e depois a **segunda forma** (a posição extra/invisível). Só então entre em batalha com 1P.
7. Pause a batalha e olhe o endereço `08B48AE8` (ID 1P). **O resultado que buscamos é `00`**, enquanto o personagem visual é Goku Base. Também confira se o Goku original (ID 03) continua intacto.
8. Para desfazer o experimento, volte à seleção, abra o menu extra e pressione **Quadrado sozinho** (sem L/R). O plugin tenta restaurar as cinco entradas originais; confirme `BRIDGE DESARMADA` no diagnóstico. **Desarme apenas na seleção, nunca no meio da batalha**. Reiniciar totalmente o jogo também volta ao EBOOT base.

Nota: a Color escolhida no plugin ainda **não é aplicada à batalha**. Nesta V0.9 use a Color 1 no seletor original. O X não envia automaticamente a seleção para o jogo: ele prepara o slot de Videl como ponte de teste. Isso evita adivinhar onde modificar a escolha de equipe.

Se o ID 00 travar, entre em loop ou carregar personagem diferente, pare e mande o diagnóstico e print do debugger. O objetivo desta etapa é **testar a identidade 00**, não prometer uma nova forma final.

## Operação e segurança

O código valida o estado original de **todas as cinco entradas ANTES de escrever**, recusa patches se outro cheat ou executável divergente estiver ativo, atualiza caches de dados/instruções e restaura somente os valores que ainda sejam de sua autoria. Isso não elimina riscos de incompatibilidade e travamentos; teste apenas com progresso salvo e com backup.

No menu, **Goku Mid 01** continua somente com prévia, bem como a seleção de 2P–4P. O plugin V0.9 não injeta arquivos em disco, não toca no PACKFILE e não registra personagens automaticamente.

## Compilar

Atualize o repositório e execute **Actions → Compilar PRX do menu**. Se der certo, baixe `TTT-Expansion-PRX-V090`, contendo a pasta instalável `TTT_EXPANSION`. Instale em `PSP/PLUGINS/`. O PSPSDK não está disponível aqui; rode o workflow para validar a compilação PSP e a execução real.

## Próximas etapas

Se a ponte 00 funcionar: investigar `01` separadamente, carregar CHR próprio da entrada 1194 ou redirecionar o carregador a `ms0:/PSP/PLUGINS/TTT_EXPANSION/characters/...` e associar a seleção do menu extra ao slot ativo sem exigir Videl como intermediária.
