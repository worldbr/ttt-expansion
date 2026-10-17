# TTT Expansion Plugin V0.3.3 — Diagnóstico de framebuffer

O usuário confirmou que o PRX V0.3.2 **carregou no jogo**, detectou L+R+Quadrado e registrou abertura/fechamento do menu, mas nada apareceu na tela. Esta versão investiga somente o desenho.

## Mudanças

- Indicador de diagnóstico maior no canto superior esquerdo (`TTT V033 LOADED`), além do rodapé original.
- Consulta **dois framebuffers** com `sceDisplayGetFrameBuf`, parâmetros `PSP_DISPLAY_SETBUF_NEXTHSYNC` (CUR) e `PSP_DISPLAY_SETBUF_NEXTVSYNC` (NEXT).
- Quando seguros, tenta desenhar em ambos; se forem iguais, desenha uma só vez.
- Em `diagnostico.txt`, grava `CUR` e `NEXT` com retorno da API, endereço, stride, formato, código de validação, tentativa de desenho e se o menu está aberto.
- Logs iniciais, ao alternar o menu e periodicamente a cada 240 frames (cerca de 4 segundos).
- Nenhum patch no jogo, no ID A0, cheats ou PACKFILE.

## Instalando

1. Atualize o repositório GitHub com o **pacote de arquivos alterados** desta V0.3.3 (preserve os caminhos).
2. GitHub Actions → **Compilar PRX de diagnostico** → Run workflow.
3. Baixe o artifact `TTT-Expansion-PRX-V033` e extraia a pasta `TTT_EXPANSION` em `PSP/PLUGINS`, substituindo o `.prx` antigo.
4. Reinicie completamente o PPSSPP (não use savestate).
5. Procure `TTT V033 LOADED` no canto superior esquerdo. Pressione **L + R + Quadrado** duas vezes.
6. Abra `PSP/PLUGINS/TTT_EXPANSION/diagnostico.txt`. Envie **as linhas V0.3.3 `CUR` e `NEXT`** e informe se o indicador apareceu.

## Interpretando o log

Exemplo (fictício): `CUR rc=0x00000000 fb=0x04000000 stride=512 fmt=3 check=0 drawn=1 open=1`

- `rc != 0`: chamada falhou.
- `check=0` e `drawn=1`: escrita ao framebuffer foi *tentada*, não significa que saiu na tela. O caminho de renderização do PPSSPP pode ignorar/cobrir escrita da CPU.
- `check=1`: ponteiro nulo. `2`: formato não reconhecido. `3`: stride fora do limite. `4`: endereço fora da VRAM esperada. `5`: retângulo ultrapassaria VRAM.
- Se `drawn=1` em CUR/NEXT, mas nada aparece, o próximo passo é pesquisar/hookar o ponto correto no pipeline de apresentação da GE, em vez de insistir em escrita direta da CPU.

**Não use como plugin final**: o desenho simultâneo nos buffers pode causar artefatos visuais ou piscar. É um experimento temporário. Se travar, remova a pasta do plugin e reinicie.

## Compilação local opcional

`gcc -std=c11 -Wall -Wextra -Werror -Isrc src/overlay.c src/toggle.c src/framecheck.c tests/test_native.c -o test_native && ./test_native`

Não há PSPSDK neste ambiente; compilação do PRX e comportamento visual no PPSSPP não puderam ser validados aqui.
