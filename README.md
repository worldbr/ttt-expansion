# TTT Expansion - PRX Diagnostico V0.3

Primeiro experimento de plugin **PPSSPP-only** para **Dragon Ball Z: Tenkaichi Tag Team** (`ULUS10537`).

## Estado: codigo-fonte, ainda sem PRX comprovado

* O projeto tem `module_start`, `module_stop`, thread de diagnostico, arquivo de log e painel grafico **basico** no framebuffer.
* Reconhece `L + R + Quadrado` (`0x8300`) por transicao; nao repete segurando.
* **Nao modifica a ISO, EBOOT, cheat, ID A0 ou PACKFILE.**
* Nao ha `ttt_expansion.prx` compilado neste ZIP, nem teste em PPSSPP realizado. O SDK nao esta neste ambiente.
* O arquivo HTML V0.2 e apenas uma demonstracao web separada. A interface detalhada ainda precisara ser portada para o pipeline grafico PSP.

## Compilacao com PSPDEV instalado

Use terminal Linux/WSL2 com PSPDEV/PSPSDK. Na raiz do projeto:

```sh
export PSPDEV=/usr/local/pspdev  # ajuste ao local instalado
export PATH="$PSPDEV/bin:$PATH"
make
```

Deve gerar `ttt_expansion.prx`. A opcao `BUILD_PRX=1` e o arquivo `exports.exp` estao incluidos.

## Compilacao via GitHub Actions

1. Crie um repositorio seu no GitHub.
2. Copie **o conteudo da pasta deste projeto** para a raiz do repositorio, incluindo `.github/workflows/compilar-prx.yml`.
3. Execute em **Actions -> Compilar PRX de diagnostico -> Run workflow**.
4. Se houver sucesso, baixe o artifact `TTT-Expansion-PRX-V03`.
5. O artifact contem `TTT_EXPANSION_INSTALAR.zip` com `TTT_EXPANSION/plugin.ini` e `TTT_EXPANSION/ttt_expansion.prx`.
6. Caso falhe, compartilhe o log do build para diagnostico. O workflow ainda nao foi rodado neste ambiente.

Usa a imagem oficial `pspdev/pspdev:v20261001` e o Makefile baseado no fluxo PSPSDK documentado.

## Instalacao no PPSSPP (depois de compilar)

```text
MemStick/
  PSP/
    PLUGINS/
      TTT_EXPANSION/
        plugin.ini
        ttt_expansion.prx
        diagnostico.txt   <- aparece depois, se o plugin conseguir escrever
```

Em Windows, use a funcao **Mostrar pasta Memory Stick** do PPSSPP para encontrar o diretorio exato; nao presuma que fica na pasta do executavel. Verifique plugins habilitados. Sempre reinicie o jogo sem savestate antigo.

## Teste esperado

1. Inicie o jogo americano `ULUS10537`.
2. Localize o texto pequeno **TTT EXP PRX V03** na borda inferior esquerda.
3. Durante a tela de escolha de personagem, aperte `L + R + Quadrado`.
4. Deve aparecer um painel azul com `TTT EXPANSION / PLUGIN PRX RUNNING`.
5. Pressione novamente para fechar.
6. Em caso de ausencia de desenho, veja `diagnostico.txt` dentro da pasta do plugin.

### Limitacoes importantes

- Overlay de diagnostico desenha diretamente no framebuffer retornado por `sceDisplayGetFrameBuf`. Dependendo do backend grafico do PPSSPP e de quando o jogo apresenta seus quadros, **o painel pode piscar, ser coberto ou nao aparecer**.
- A renderizacao e o reconhecimento da combinacao rodam em todo o jogo nesta versao, **nao so no menu de selecao**. Use apenas para diagnostico.
- O plugin apenas faz `sceCtrlPeekBufferPositive`; **nao bloqueia o input do jogo**.
- Nenhum acesso aos enderecos do projeto de expansao foi incluido, para evitar alterar memoria de um EBOOT diferente.
- Escreve diretamente em framebuffer em VRAM apenas se o endereco, formato e limites parecerem validos.
- O log pressupoe que o PPSSPP mapeie corretamente o caminho `ms0:/PSP/PLUGINS/TTT_EXPANSION/diagnostico.txt`.
- Se travar, remova a pasta `TTT_EXPANSION`, reinicie PPSSPP e reporte se houve log. Usar backup dos saves.

## Verificacao no PC (sem SDK)

```sh
gcc -std=c11 -Wall -Wextra -Werror -Isrc src/overlay.c src/toggle.c tests/test_native.c -o teste
./teste
```

## Referencias

* https://www.ppsspp.org/docs/reference/plugins/
* https://pspdev.github.io/techniques/prx_modules.html
* https://github.com/pspdev/pspsdk/blob/master/src/samples/prx/testprx/Makefile.sample
* https://pspdev.github.io/pspsdk/pspdisplay_8h.html
