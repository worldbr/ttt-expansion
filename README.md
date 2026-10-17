# TTT Expansion V0.9.1 — GPU Guard + teste ID00 (ULUS10537)

Esta versão **combina o código gráfico da V0.8.2 (corrigiu a tela preta em seu teste)** com a ponte experimental da V0.9.

## O que está comprovado e o que é experimental

- Já confirmado por teste do usuário na V0.8.2: a tela de batalha voltou ao normal depois de abrir e fechar o menu, graças ao contexto GPU salvo/restaurado.
- Implementado para V0.9.1, **ainda não testado no PPSSPP**: usar a entrada experimental da Videl para tentar carregar **Goku Early, ID externo 00**, com recursos emprestados do Goku TTT. **Não é um novo CHR nem um Mod Loader**.
- Goku Mid ID 01 e as escolhas 2P–4P continuam **somente prévia**.
- O patch ID00 modifica cinco locais do EBOOT na RAM, somente após X, e recusa a instalação se os valores não coincidirem.
- A seleção utiliza provisoriamente **Videl forma 2** e pode falhar em batalha por tabelas de personagens ainda não expandidas. Faça backup, desative cheats antigos e não use saves importantes.
- Com o menu aberto, **Quadrado (sozinho)** tenta desfazer a ponte. Não desarme durante a batalha.
- Atalho para abrir/fechar: **L + R + Quadrado**. Círculo fecha, X arma ID00 se P1/Goku Early, setas e L/R navegam.

## Como compilar

Atualize os arquivos no repositório de GitHub com os mesmos caminhos, commit e **Actions → Compilar PRX do menu → Run workflow**.
Artifact esperado: `TTT-Expansion-PRX-V091`. Copie o conteúdo de `TTT_EXPANSION` do artifact para `PSP/PLUGINS/TTT_EXPANSION` do PPSSPP.

**Sem PSPSDK disponível neste ambiente**: verificações C nativas e fontes foram validadas, não o PRX no emulador. Os resultados finais dependem do seu teste.

## Teste em duas etapas (não pule a primeira)

**A. Compatibilidade gráfica:** reinicie completamente o PPSSPP, com a ISO correta e sem cheats de expansão. Na seleção de personagens abra o overlay com L+R+Quadrado, **não pressione X**, feche com Círculo e entre numa batalha usando **Goku original ID03**. Confirme cenário 3D normal. Procure no `diagnostico.txt` as linhas `[TTT V0.9.1] GE GUARD INIT` e `GE GUARD CLOSE`.

**B. Ponte ID00 (somente se A passou):** reinicie completamente o PPSSPP. Na seleção, abra o menu, escolha **1P e Goku Early**, pressione **X** uma vez; confira `[TTT V0.9.1] BRIDGE CONFIRMAR installed=0x00000001 error=0x00000000`. Feche com Círculo, escolha **Videl, forma experimental 2** no jogo e entre na batalha. Verifique `08B48AE8` no Memory Viewer para o ID externo `00`; observe cenário, modelo e possíveis travamentos. **Não confirme batalha caso a segunda forma não exista.**

Se o jogo travar ou ficar preto, feche o PPSSPP e restaure a versão V0.8.2. A instalação da ponte não autoriza concluir que o ID00 é totalmente suportado pelo motor do TTT.

## Arquitetura futura

Antes de carregar CHR/SFX externos via `sceIoOpen`, é necessário validar a identidade externa ID00, seu caminho de resolução CHR e as dependências internas da batalha. Reuso de PACKFILE existente é o teste mais seguro para essa etapa.
