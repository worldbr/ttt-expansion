# TTT Expansion V0.9.8.7 — Menu Recovery (experimental)

Base: V0.9.8.6, com IDs 00/01/02, 4 Colors por personagem e seleção 1P–4P preservados.

## Problema investigado
A interface pode desaparecer ao percorrer personagens / jogadores e só voltar depois de reiniciar. A causa real ainda não foi verificada em `diagnostico.txt` do incidente. Duas possibilidades identificadas no fonte: fechamento involuntário pelo hotkey de três botões e bloqueio persistente de desenho ao ocorrer uma falha transitória de GPU.

## Alterações
- O atalho **L + R + Quadrado** agora exige uma NOVA pressão de Quadrado com L e R segurados (ou os três apertados de uma vez). Manter Quadrado pressionado e depois usar L/R para alternar jogadores não fecha o menu.
- O GPU Guard continua impedindo recuperação quando `sceGeSaveContext` ou `sceGeRestoreContext` falha.
- Se houver um erro transitório de comando GU, o worker tenta recuperar o desenho **no máximo duas vezes**, uma tentativa a cada 120 frames, nunca com GPU Guard em estado de falha.
- Log adicional a cada 120 frames com `MENU WATCHDOG` e `MENU RECOVERY RESULT`. `MENU HOTKEY FECHADO` significa que o atalho realmente fechou a interface.

## Teste no PPSSPP
1. Manter ISO/PACKFILE `0x3734` e EBOOT atual. **Não reconstruir a ISO.**
2. Compilar o projeto **completo** pelo workflow no GitHub Actions. Instalar o PRX, reiniciar PPSSPP.
3. Abrir menu com L + R e pressionar Quadrado novo. Navegar 1P→2P→3P→4P, ir e voltar com L/R, mudar personagens e Colors.
4. Se o menu sumir, **não reiniciar imediatamente**: espere 3–5 s e pressione L+R+Quadrado (com Quadrado solto antes). Salve `diagnostico.txt` após o teste.
5. Envie linhas `MENU WATCHDOG`, `MENU HOTKEY`, `MENU RECOVERY RESULT`, `HOOK`, `GE GUARD` e `INPUT`.

## Limitação
Os testes no PC verificam código básico, mas não substituem execução PSPSDK/PPSSPP; esta correção ainda é experimental. Os IDs 01/02 continuam temporariamente com a forma interna 0x47, e os SFX continuam reservados.
