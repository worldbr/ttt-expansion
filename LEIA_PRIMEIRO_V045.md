# TTT Expansion V0.4.5 — confirmar personagens depois da luta

**Uso exclusivo no PPSSPP, ULUS10537.** Atualização SOMENTE do PRX: não altere o EBOOT V0.4, o PACKFILE `0x37FC`, os CHRs nem `characters.txt`.

## Diagnóstico confirmado

Em `diagnostico(7).txt`, a primeira batalha confirmou a injeção dos quatro personagens: J1=01, J2=36, J3=39, J4=3C. O plugin liberou as seleções da rodada anterior e o contador do PACKFILE permaneceu 0x37FC. Porém, depois da batalha, o código do resolvedor **checked** em `0x089C6CA0` teve dois comandos alterados: palavra 0 (`0x682B470E` no novo log, antes `0x682BE5B6`) e palavra 9 (`0x682B4776`), enquanto o EBOOT V0.4 espera `0x2C8800A1` e `0x246343E0`. A recuperação da V0.4.4 recusou a confirmação com erro `-42` pois somente conhecia a palavra 0.

## O que mudou

- O PRX guarda a informação de ter observado **uma imagem integralmente intacta** dos dois resolvedores do EBOOT V0.4 no mesmo processo.
- Quando você abre o plugin ou confirma com X, a recuperação só considera **as duas posições observadas (0 e 9)**, quando a instrução nelas tem assinatura `0x682Bxxxx`. Não repara posições adicionais nem altera o resolvedor unchecked.
- Antes de escrever, todas as outras instruções são comparadas à imagem verificada do EBOOT V0.4 e o contador do PACKFILE deve continuar `0x37FC`.
- Restaura as duas palavras juntas, limpa caches de dados/instruções do PSP e revalida as instruções. Se houver diferença não reconhecida, interrompe a recuperação com log — não desliga as verificações.
- Os 47 IDs, as quatro Colors, J1–J4, os retratos e nomes editáveis são preservados. Nenhum CHR/SFX foi reescrito.

## Instalação e teste

1. Faça backup da ISO e dos arquivos anteriores que funcionaram na primeira batalha.
2. Substitua os arquivos do repositório pelos que estão no ZIP `TTT_EXPANSION_MENU47_V045_GITHUB_UPDATE.zip` e execute o workflow de compilação no GitHub.
3. Substitua apenas `ms0:/PSP/PLUGINS/TTT_EXPANSION/ttt_expansion.prx` pelo PRX recém-compilado, ou extraia a pasta completa do artefato para o diretório correto. Confirme que não há duas versões ativas do plugin.
4. Reinicie o jogo do começo (não carregue um savestate criado com o PRX anterior). No log deverá aparecer `TTT MENU47 V0.4.5`.
5. Selecione personagens extras para J1–J4, confirme com X no plugin, feche, confirme o portador no seletor original, entre em batalha e volte à seleção. Abra o plugin, escolha personagens **diferentes**, confirme e tente uma segunda batalha.

## Interpretando o log

- `CODE_RECOVERY rc=0x00000001 mask=0x00000201 trusted=0x00000001`: os dois comandos foram restaurados com a lista branca do caso do log 7.
- `CODE_RECOVERY rc=... trusted=0x00000000`: nenhuma imagem original foi observada no processo; a recuperação foi recusada.
- `CONFIRMAR FALHOU rc=0xFFFFFFD6`: verificação do EBOOT ainda bloqueou; envie as linhas `CODE_RECOVERY`, `NATIVE_GUARD` e `OPEN NATIVE`.
- `ACTOR_RESULT`: o ID foi gravado na RAM na batalha anterior; o log também deve mostrar IDs da segunda batalha para provar reutilização real.

**Aviso experimental:** esses endereços parecem ser sobrescritos por dados de outra cena depois da batalha. Restaurá-los mesmo com assinatura conhecida e verificação pode causar conflito se o jogo continuar usando essa região para outra finalidade. Portanto, faça testes com backup e interrompa se notar travamentos ou gráficos corrompidos. O teste local valida apenas a lógica de decisão; ainda não houve compilação PSPDEV ou execução no PPSSPP neste ambiente.
