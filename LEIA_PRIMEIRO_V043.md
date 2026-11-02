# TTT Expansion MENU47 V0.4.3 — segunda batalha + 47 IDs

## O que o diagnostico(5).txt realmente demonstra

- Há execuções antigas com **"47 slots (3 habilitados)"** e execuções V0.4/V0.4.1 com **"47 slots"**. O arquivo concatena várias inicializações; não é uma única execução.
- Na V0.4.1 o plugin registra `NOVA RODADA` após a luta, mas X falha com `rc=0xFFFFFFD6` (-42) para `0x4A` e `0x01`, apesar de `pack_count=0x37FC`.
- A falha -42 vem da checagem do resolvedor nativo EBOOT (assinaturas e/ou tabela). O log V0.4.1 **não informa qual verificação** falhou.
- O log **não mostra** `MENU47 V0.4.2` ou `OPEN NATIVE`, então não comprova que o PRX V0.4.2 foi instalado/testado.
- Encontramos um teste desatualizado (`tests/test_skin_assets.py`) e um `cp` de arquivo inexistente (`LEIA_PRIMEIRO_REBUILD_V2.md`) no workflow da V0.4.2, capazes de impedir que o GitHub entregue um PRX novo.

## Correções V0.4.3

1. **47 slots e IDs preservados**, 4 jogadores, 4 Colors e nomes editáveis em `TTT_EXPANSION/characters.txt`. Nenhum limite de três IDs no seletor.
2. **Verificação/recuperação individual por ID:** quando uma tabela retorna a `0xFF` depois da batalha, o plugin pode restaurar os marcadores do personagem selecionado. Um conflito em outro dos 47 IDs não impede a confirmação deste ID.
3. **Auditoria parcial segura na abertura:** tenta restaurar somente bytes `0xFF` nas 47 entradas reservadas, desde que ambas as assinaturas do EBOOT V0.4 e o contador `0x37FC` sejam válidos. Entradas com bytes desconhecidos ficam intactas e são registradas como conflitos.
4. **Sem sobrescrever código desconhecido:** se a rotina do EBOOT não for a V0.4 esperada, X falha com diagnóstico explícito; não faz patch perigoso.
5. **Ciclo pós-batalha:** mantém limpeza de confirmações da rodada anterior quando o shim registra escrita no ator, preservando escolhas/cores como preferência visual.
6. **Workflow reparado:** remove a cópia de arquivo ausente, atualiza o teste antigo, identifica o PRX como `MENU47 V0.4.3` e inclui verificações de empacotamento.
7. **Verificador offline de EBOOT e PACKFILE:** `tools/verificar_instalacao_v043.py` confirma assinaturas, 47 IDs e 235 índices sem modificar arquivos.

## Instalação — atenção para não voltar à versão de três Gokus

1. Faça backup do PRX e da ISO atual. Se você já tem **EBOOT V0.4 + PACKFILE 0x37FC** funcionais, **não substitua esses arquivos**. Esta atualização muda apenas o PRX e a lógica de confirmação.
2. No GitHub, envie os arquivos do ZIP `..._V043_GITHUB_UPDATE.zip`, mantendo `src/`, `tests/`, `.github/`, `tools/`, `Makefile` e este README. Ou substitua o repositório pelo pacote completo V0.4.3.
3. Execute **Actions > TTT Expansion Menu 47 - PRX > Run workflow** e confira que o build terminou com `COMPILACAO CONCLUIDA`.
4. Baixe o artefato `TTT-Expansion-Menu47-PRX` do workflow **V0.4.3**. Instale **somente o novo `ttt_expansion.prx`** em `PSP/PLUGINS/TTT_EXPANSION/`. Não copie um PRX de artefato antigo por cima.
5. Preserve seu `characters.txt` personalizado e seus recursos CHR/SFX existentes.
6. **Reinicie o jogo completamente**, não apenas carregue save state. Opcionalmente renomeie o `diagnostico.txt` antigo antes de testar, para não misturar versões.

## Teste decisivo no PPSSPP

1. Abra o jogo e o plugin com **L + R + Quadrado**. A primeira linha do log novo deve ser `[TTT MENU47 V0.4.3] module_start OK` e a abertura deve registrar `OPEN NATIVE`.
2. Confirme com **X** um dos 47 slots (ex.: slot 37, ID 0x4A) no plugin, feche com **BOLA** e marque **PRONTO no seletor original** com X. O PRX não marca READY sozinho.
3. Entre em batalha, termine-a, retorne à seleção e repita com outro personagem (ex.: ID 0x01). Verifique se a confirmação persiste.
4. Se falhar, envie o **novo** `diagnostico.txt`, especialmente `OPEN NATIVE`, `NATIVE_GUARD` e `CONFIRMAR FALHOU`. Os erros distinguem `BAD_CODE`, `BAD_COUNT`, `CONFLICT` e `VERIFY_FAILED`.
5. Para conferir os arquivos de disco, use no Windows: `py tools/verificar_instalacao_v043.py --elf "CAMINHO\\EBOOT.ELF" --header "CAMINHO\\PACKFILE_CABECALHO.BIN"`.

## Sobre os "47 personagens"

A V0.4 disponibiliza **47 IDs de batalha** e os respectivos grupos de cinco recursos no cabeçalho. **Apenas os três primeiros possuem CHR próprios confirmados nesta base**; os outros 44 são placeholders/aliases com dados de Goku até serem substituídos por modelos, SFX e golpes individuais. Isso não significa 47 personagens inéditos completos. O pacote de referência do EBOOT/header V0.4 (quando incluído) serve apenas para comparar/recuperar a estrutura, **não para sobrescrever uma ISO customizada automaticamente**.

## Validação realizada

Testes C locais com `gcc` passaram para 47 IDs, restauração por slot, isolamento de conflitos, recusa de assinaturas desconhecidas, 4 jogadores e 4 Colors. Testes Python de assets, 47 nomes e workflow também passaram. **Ainda não há PRX compilado neste ambiente nem teste executado no PPSSPP**. A correção em jogo depende do resultado do novo diagnóstico.
