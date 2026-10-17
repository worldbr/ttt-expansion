# TTT Expansion - V0.4 (EXPERIMENTO GE/GU)

## O que os logs anteriores provam
- PRX carrega e o atalho **L + R + Quadrado** funciona.
- `drawn=1 after=1` prova gravação na VRAM, **não apresentação na tela**.
- `before=0` frequentemente sugere que a imagem foi sobrescrita ou lida de outro render target.

## Esta versão
Faz uma **tentativa experimental de desenhar retângulos azuis e amarelos pela GE/GU** do PSP. Nenhum arquivo do PACKFILE ou dado de personagem é alterado.

**Atenção:** a GU é compartilhada com o jogo. Esta versão é apenas um teste de diagnóstico: pode gerar glitches, travamento ou queda de desempenho. Salve seu progresso antes. Se ocorrer problema, desabilite o plugin no PPSSPP e restaure a V0.3.4.

## Compilar
Substitua os arquivos do repositório pelo conteúdo deste ZIP, preservando a pasta `.github`. Execute GitHub Actions `Compilar PRX de diagnostico`. O artifact esperado é `TTT-Expansion-PRX-V040`. Não há um PRX já compilado neste pacote.

## Testar no PPSSPP
1. Coloque `ttt_expansion.prx` e `plugin.ini` em `PSP/PLUGINS/TTT_EXPANSION/`.
2. Habilite plugins, reinicie inteiramente o jogo e vá ao menu de personagens.
3. Pressione L + R + Quadrado. Aguarde até 5 segundos. **Esperado:** retângulos azuis e amarelos, sem texto por enquanto.
4. Consulte `diagnostico.txt` para `GPU INIT rc=0` e `GPU DRAW rc=0`. Mesmo com rc=0, o desenho pode ainda ser invisível.
5. Se congelar, reinicie sem o plugin e relate o que ocorreu.

## Primeiro teste recomendado, sem trocar plugin
Antes de instalar esta versão, experimente no PPSSPP **Graphics > Software rendering**, se disponível (isso reduzirá bastante o desempenho). Com a V0.3.4, veja se as barras aparecem. Isso pode discriminar a renderização por GPU do acesso à RAM. Depois volte ao backend normal.

## Limitações
Ainda não há gancho na etapa de composição/presentação. Este é um teste GE/GU, não uma interface pronta nem garantia de compatibilidade com todos os backends.
