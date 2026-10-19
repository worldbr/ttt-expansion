# TTT Expansion V0.9.6 — rastreamento do pré-carregamento

**Jogo:** Dragon Ball Z: Tenkaichi Tag Team, PSP ULUS10537 (EBOOT.OLD original usado nos testes).
**Status:** implementação de diagnóstico, **NÃO faz substituição automática de qualquer slot original pelo personagem configurado no Expansion**.

## Objetivo

Nosso plano é que, independentemente do personagem escolhido na seleção ORIGINAL, o **TTT Expansion** possa fornecer ID e Color separados para 1P, 2P, 3P e 4P **ANTES** de o jogo carregar a batalha. A V0.9.5 confirmou leituras dos quatro campos `ACTOR RAW` durante combate, mas não capturou chamadas no hook de resolução de recursos naquela batalha (`checked=0`). Por isso ainda precisamos localizar **qual instrução copia/prepara a seleção para a batalha**.

A V0.9.6 continua a observação sem sobrescrever os quatro endereços e sem inventar um vínculo entre `LOAD TRACE site` e jogador.

## Novidades da V0.9.6

- **`ACTOR CHANGE`**: amostra os quatro pares ID+Color a cada quadro de VBlank e registra **somente mudanças**, incluindo `old_id`, `id`, `old_color`, `color`, `player`, `frame`, `plan` e `menu`. O primeiro evento por jogador é uma **amostra inicial** (`initial=1`), não comprova que houve alteração.
- Buffer circular de 128 eventos, sem I/O na amostragem; o worker descarrega até 32 entradas a cada 30 quadros. Se houver transbordamento, registra `ACTOR GAP`.
- `PLAN CONFIRMAR ... frame=...`: correlaciona as preferências do Expansion com a linha do tempo dos jogadores.
- `tools/analisar_timeline.py`: leitura opcional do `diagnostico.txt` e resumo da linha do tempo, sem dependências adicionais.
- `docs/BREAKPOINT_PPSSPP.md`: guia para encontrar o PC da escrita real nos campos dos jogadores.
- Mantém GPU Guard, menu, retratos, seleção 1P–4P e ponte do ID00/CHR1194, **com o mesmo caráter experimental da V0.9.5**.

## Passo a passo do teste seguro

1. Faça backup do PRX anterior (V0.9.3 comprovada para CHR; V0.9.5 para diagnóstico). Mantenha seu PACKFILE modificado com CHR na entrada 1194.
2. Substitua os arquivos do projeto e compile no GitHub Actions (**Compilar PRX do menu**). Artifact esperado: `TTT-Expansion-PRX-V096`.
3. Instale o PRX, **reinicie o PPSSPP** e confirme no `diagnostico.txt` a linha `module_start OK` da V0.9.6.
4. Na seleção, abra nosso menu com **L+R+Quadrado**, marque **1P Goku Early, Color 1** e confirme com **X**.
5. Feche com **Círculo**, selecione **qualquer personagem original** (por exemplo Piccolo, e não necessariamente a quinta forma do Goku) e entre em batalha.
6. Copie as primeiras e últimas linhas `ACTOR CHANGE` do teste e também `PLAN CONFIRMAR`, `BRIDGE CONFIRMAR`, `LOAD TRACE` se existirem e `ACTOR RAW`. Diga qual personagem o seletor ORIGINAL escolheu para cada jogador.
7. Para a investigação que identifica o **código que escreve o ID**, siga o procedimento em `docs/BREAKPOINT_PPSSPP.md` e registre o endereço PC da primeira escrita relevante em `0x08B48AE8`.

### Como ler o registro

```text
[TTT V0.9.6] ACTOR CHANGE seq=0x00000005 frame=0x00000340 player=0x00000001 initial=0x00000000 old_id=0x00000023 id=0x00000000 old_color=0x00000000 color=0x00000000 plan=0x00000001 menu=0x00000000
```

**EXEMPLO, não é um teste real.** `frame` é o contador relativo desde o início da thread. `plan=1` indica que o plano do 1P estava confirmado; **não significa `applied=1`**. `menu=0` indica que o menu do plugin estava fechado. Uma mudança durante a batalha pode corresponder a transformação/respawn, não necessariamente ao primeiro carregamento.

Para resumir um log no Windows (com Python disponível):

```bat
python tools\analisar_timeline.py diagnostico.txt
```

## Limitações e cuidados

- Este plugin **não substitui** o ID de jogadores na preparação da batalha. A seleção original ainda é a que define os jogadores, exceto pelo caminho experimental da quinta forma já existente na V0.9.5.
- Não confunda endereço observado em **combate** com endereço de pré-carregamento. O detector por quadro pode perder transições rápidas entre dois VBlanks; por isso recomendamos breakpoint de escrita no emulador.
- `LOAD TRACE site` é um **local de chamada de função**, e **não um identificador de jogador**.
- Não modifique manualmente os endereços de jogadores durante o carregamento. Se ocorrer travamento, reinicie o PPSSPP e restaure o PRX da V0.9.3.
- Compilação nativa dos módulos/testes foi feita no PC; build PSPSDK + execução em PPSSPP precisam de confirmação do usuário.

## Próxima fase

Após identificar a instrução/rotina que escreve ID+Color e sua associação com 1P–4P, poderemos estudar um **hook no instante anterior ao carregamento**, com validação de opcode e restaurabilidade, começando por **apenas 1P**. O processamento de CHR 1194 deverá continuar restrito ao ID00 Color1 até comprovar suporte a outros trajes e personagens.
