# TTT Expansion Menu V0.6 — primeiro menu gráfico navegável

**Protótipo para PPSSPP / ULUS10537.** Desenvolvido a partir do hook V0.5.3 que o usuário confirmou ficar visível. Mantém os três `jal` verificados (`08A63A48`, `08A63B10`, `08A6408C`) e o atalho **L + R + Quadrado**.

## Novidade

A V0.6 substitui os retângulos de teste por um seletor azul e amarelo renderizado no jogo, com fonte própria 5×7 desenhada pela GE. Inclui dois cartões para **Goku Base** (`recurso 00`) e **Goku SSJ1** (`recurso 01`) sob ID externo de teste **A0**; escolha de Color 1–4; abas Personagens, Mapas (reservada) e Ajustes; textos em português/inglês/espanhol.

**Segurança:** este é um protótipo de INTERFACE. Confirmar com X escreve apenas uma linha `CONFIRMACAO PREVIA` em `diagnostico.txt` com os valores selecionados. **Não modifica o ID A0, não carrega recursos novos e não altera o PACKFILE ou EBOOT no disco.** Para isso precisaremos integrar a confirmação às rotinas de seleção de personagem numa próxima versão.

## Controles

- **L + R + Quadrado**: abre/fecha menu extra;
- **Esquerda / Direita**: seleciona Goku Base ou Goku SSJ1;
- **Cima / Baixo**: seleciona Color 1 a 4;
- **Triângulo**: alterna entre Personagens / Mapas / Ajustes;
- **Select**: alterna entre português / inglês / espanhol;
- **X**: testa confirmação, registrada no log, sem alterar o jogo;
- **Círculo**: fecha o menu e restaura as chamadas JAL originais.

## Instalar pelo GitHub

1. Faça backup da V0.5.3, que está funcionando.
2. Extraia o ZIP *ATUALIZACAO* e substitua os arquivos na mesma estrutura de pastas do repositório. Não envie a pasta principal inteira como subpasta.
3. Commit e **Actions > Compilar PRX do menu > Run workflow**. Artifact esperado: `TTT-Expansion-PRX-V060`.
4. Extraia `TTT_EXPANSION/` para `PSP/PLUGINS/` do PPSSPP substituindo o PRX anterior.
5. Reinicie PPSSPP inteiramente e abra ULUS10537. Na tela de seleção pressione L+R+Quadrado.
6. Navegue, aperte X e procure `CONFIRMACAO PREVIA` no arquivo `PSP/PLUGINS/TTT_EXPANSION/diagnostico.txt`.

## Nota técnica e riscos

Faz um hook experimental em três JALs na RAM e usa GE/GU durante o evento de troca do framebuffer, como a V0.5.3. Fonte e UI usam sprites do GE, com lista alinhada e 128 KiB de comandos. O menu não injeta entradas no seletor normal do jogo. Pode ainda causar artefatos gráficos, lentidão ou travamento. Salve o progresso e teste sem savestates; remova o PRX se ocorrer problema. O ambiente do assistente **não possui PSPSDK**; a compilação do PRX e o comportamento em emulador ainda precisam ser validados pelo GitHub/PPSSPP.

## Testes nativos disponíveis

```bash
cc -std=gnu11 -Wall -Wextra -Isrc tests/test_menu_state.c src/menu_state.c -o /tmp/test_menu && /tmp/test_menu
cc -std=gnu11 -Wall -Wextra -Isrc tests/test_callsite_plan.c src/callsite_plan.c -o /tmp/test_calls && /tmp/test_calls
cc -std=gnu11 -Wall -Wextra -Isrc tests/test_native.c src/toggle.c src/overlay.c src/framecheck.c -o /tmp/test_fb && /tmp/test_fb
```
