# TTT Expansion Menu 47 — V0.1 (FONTE, não PRX compilado)

## O que está implementado

- Menu minimalista em 480×272 sem abas: 47 células numa grade de 8 colunas por 6 linhas (a 48ª posição fica vazia).
- J1–J4 com seleção/cor/confirmado independentes.
- Quatro Colors por slot, foco selecionado com TRIÂNGULO.
- Áudio opcional próprio (loop de quatro notas simples + efeitos de navegação/confirmar/erro); sintetizado a 44,1 kHz numa thread separada via `sceAudioChReserve` e `sceAudioOutputBlocking`; sem usar canções protegidas ou arquivos extras.
- SELECT liga/desliga **a música do plugin**; os efeitos sonoros de botões continuam ativos.
- Se os canais de áudio estiverem todos ocupados pelo jogo, o áudio não será reproduzido; menu permanece funcional.
- A fonte de nome/ícone foi simplificada; os três retratos PNG/RGBA são os de Goku Early, Mid e Mid SSJ da base V2.0.3.
- Usa o hook de desenho anterior e o actor injector anterior, mas **NÃO** ativa o antigo `battle_bridge` nem `resource_hook`, para evitar conflito com o ELF nativo V0.3 testado pelo usuário.

## Situação de 47 slots

| Posição da grade | Personagem | ID de combate | Status |
|---|---|---|---|
| 01 | Goku (Início) | 00 | Experimental, depende do ELF V0.3 + PACKFILE expandido |
| 02 | Goku (Meio) | 01 | Experimental, depende do ELF V0.3 + PACKFILE expandido |
| 03 | Goku (Meio) SSJ | 02 | Experimental, depende do ELF V0.3 + PACKFILE expandido |
| 04–47 | (reservados) | não atribuído | Aparecem no menu, mas confirmar NÃO injeta ID |

**Importante:** 47 posições visuais NÃO equivalem a 47 personagens disponíveis em batalha. Para preencher os outros 44, precisamos dos IDs reais, mapeamento/CHRs e testes no EBOOT/PPSSPP. Não preencher IDs sem verificar carregamento, nomes e outros recursos.

## Teclas do plugin PSP

- Abrir/fechar menu: mantenha **L+R**, pressione **QUADRADO**.
- Cruz direcional: na grade, esquerda/direita e cima/baixo percorrem as 47 posições.
- TRIÂNGULO: alterna foco Grade/Colors.
- Cruz direcional com foco Colors: muda Color C1–C4.
- L/R individual: troca J1/J2/J3/J4.
- X: confirma somente um dos três slots funcionais.
- QUADRADO sozinho: remove injeção de IDs nos quatro jogadores.
- BOLA: fecha o menu (não apaga escolhas confirmadas).
- SELECT: liga/desliga música do plugin.

## Abrir a prévia interativa no Windows

Abra `PREVIA_INTERATIVA_MENU47.html` no navegador. Clique nos slots, Colors e jogadores. O áudio começa após um clique/tecla, por restrição do navegador. A prévia NÃO modifica o jogo.

## Como compilar PRX

O SDK do PSP **não está instalado** no ambiente de criação desta versão. Então NÃO existe `ttt_expansion.prx` neste ZIP e não afirmamos que compilou ou rodou no PPSSPP.

1. Extraia o ZIP e publique os arquivos deste projeto num repositório GitHub (ou compile localmente com PSPSDK completo).
2. Em **Actions**, execute o workflow `TTT Expansion Menu 47 - PRX`.
3. Baixe o artefato `TTT-Expansion-Menu47-PRX` com o arquivo `TTT_EXPANSION/ttt_expansion.prx` produzido pelo runner.
4. Instale num backup do Memory Stick/ISO expandida em `PSP/PLUGINS/TTT_EXPANSION/`. Mantenha seus retratos e texturas.
5. Antes de confirmar os 3 Gokus, instale EBOOT **V0.3 RESOLVER** que você já testou e use a ISO com PACKFILE `0x3734`. A compilação protege a injeção por assinaturas dos resolvedores e contagem.
6. Teste menu e áudio primeiro. Teste ID/Color dos Gokus um por vez, com save state. Comportamento em combate é **experimental e ainda não validado nesta combinação**.

**Não carregar em paralelo com outro `ttt_expansion.prx`**: substitua o arquivo em backup, em vez de deixar duas versões ativas.

## Diagnóstico

`ms0:/PSP/PLUGINS/TTT_EXPANSION/diagnostico.txt`

O áudio reserva canal com `sceAudioChReserve(-1,512,PSP_AUDIO_FORMAT_STEREO)` somente com menu aberto; ao fechar, libera canal. Não interfere intencionalmente nos samples do próprio jogo. Um canal livre não é garantido.

## Limitações atuais

- PRX ainda NÃO compilado/testado em hardware/PPSSPP nesta versão. Testes de lógica C no PC passaram.
- Os 44 slots extras estão **visíveis, mas desabilitados**, até fornecer novos recursos.
- Áudio é sintético, não MP3/ATRAC3/WAV nem música do anime.
- A interface PSP utiliza números nos slots (e retratos nos três existentes) para economizar VRAM. A prévia do navegador é uma simulação visual, não uma captura real do PSP.
- Os campos de nome do menu são internos; para permitir editar 47 nomes por TXT sem recompilar, precisaremos integrar um arquivo de roster posteriormente.
