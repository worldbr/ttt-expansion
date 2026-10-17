# TTT Expansion V0.8.2 — teste de preservação do contexto GE

**Esta é uma versão de diagnóstico da GPU, baseada na V0.8.1 e sem ponte de IDs.**

## Suspeita encontrada no PSPSDK
`sceGuInit()` emite uma lista que redefine diversos registradores da GE, incluindo textura,
profundidade e estado do framebuffer. A V0.8.1 a chamava ao abrir o overlay e não restaurava
seu estado. Também executava listas GU a cada quadro sem restaurar o contexto do jogo.
Isso é compatível com cenário 3D preto após fechar o menu, mas a correção precisa ser testada.

## Mudanças na V0.8.2
- Antes de `sceGuInit`, chama `sceGeSaveContext` e após a inicialização restaura com
  `sceGeRestoreContext`.
- Faz o mesmo antes e depois de cada quadro desenhado pelo overlay.
- Se uma dessas operações retornar erro, interrompe o desenho em vez de prosseguir com um
  estado GPU desconhecido.
- Escreve `[TTT V0.8.2] GE GUARD ...` no `diagnostico.txt` com retornos e contadores.
- Mantém a interface visual V0.8.1, navegação e bloqueio de controle originais.
- **NÃO ativa o ID 00**, nem modifica tabelas ou arquivos de personagem.

## Como compilar
Envie para o repositório os arquivos desta atualização com os mesmos caminhos e execute
**Actions → Compilar PRX do menu → Run workflow**. Artifact:
`TTT-Expansion-PRX-V082`. Instale na pasta `PSP/PLUGINS/TTT_EXPANSION`, incluindo portraits.

## Teste isolado
1. Faça backup do PRX anterior e desligue completamente o PPSSPP.
2. Inicie o TTT original (sem o cheat do ID A0/00) e chegue à seleção de personagens.
3. Abra e feche o TTT Expansion pelo atalho L + R + Quadrado; não confirme IDs.
4. Entre em batalha com o Goku original ID 03. Verifique cenário, efeitos e HUD.
5. Envie linhas `GE GUARD INIT`, `GE GUARD STATUS`, `GE GUARD CLOSE` e `HOOK` do log.

Se a tela preta persistir, desligue o PPSSPP, restaure o PRX anterior/desative o plugin.
**Não continue testando novas formas enquanto a GPU não estiver estável.**

O código foi verificado por testes C locais e inspeção; a compilação PSPSDK e o comportamento
no PPSSPP ainda precisam ser validados. Salvar/restaurar contexto pode, em alguns casos,
causar erro HLE ou conflito com listas GE ativas. Por segurança, não use em partidas com
progresso não salvo.
