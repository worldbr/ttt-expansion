# Diagnóstico V0.9.8.4D — restauração de baseline

Esta é uma **versão de isolamento de falhas**, não uma expansão de sete personagens jogáveis.

- A base do plugin é a V0.9.8.3, comprovada em batalha. Menu antigo com **2 opções visíveis** (Early e Mid), mas só Goku Early Color 1 realmente injetável. 
- Apenas o reconhecimento do contador do PACKFILE muda de `0x3712` para **`0x3734`**.
- O cabeçalho e os 35 registros experimentais da V0.9.8.4 estão incluídos em `EXPANSAO_7_GOKUS/`, e não são alterados.
- Seis checkpoints `ETAPA_0..ETAPA_4` foram colocados no caminho de confirmação do Goku. Se houver crash, a última etapa registrada no `diagnostico.txt` ajuda a encontrar o local do problema.
- Nenhuma modificação nos hooks de sprites, GPU, assembly do ator, bridge ou renderização da V0.9.8.3. 
- Não use o PRX V0.9.8.3 sem o contador correto esperando que injeção funcione; esta versão é específica para `0x3734`.

## Teste controlado no PPSSPP

1. Faça backup da ISO e plugins que funcionavam. Use a ISO experimental com cabeçalho `0x3734` e EBOOT cujo contador foi corrigido para `0x3734`. Não altere dados do PACKFILE para este diagnóstico.
2. Compile este projeto no GitHub Actions e instale o novo `ttt_expansion.prx` (substitua o PRX antigo, não rode duas versões).
3. Reinicie o PPSSPP totalmente, não use savestate.
4. Entre num menu; abra Expansion; confirme **1P Goku Early Color 1** com X. Depois teste batalha.
5. Caso caia, envie o `diagnostico.txt` **completo**. Confira se aparecem `ETAPA_0_INICIO_CONFIRMACAO`, `ETAPA_1_RECURSO_RETORNOU`, `ETAPA_2_BRIDGE_RETORNOU`, `ETAPA_3_INJETOR_RETORNOU` e `ETAPA_4_SLOT_RETORNOU`.
6. Também anote o momento do crash e se aparece antes de fechar a interface ou somente ao iniciar a luta.

## Como interpretar o teste

- **Funciona:** a expansão de contagem `0x3734` junto do hook clássico funciona; a regressão da V0.9.8.4 ficou em outras mudanças (UI novo e/ou validação de offsets novos). Isolar depois.
- **Falha no mesmo ponto:** o contador maior, o cabeçalho ou outro detalhe de runtime continua suspeito. O log marca a última etapa atingida; não concluir automaticamente que é um problema de CHR.

## Limitações

Não existe compilador PSPSDK neste ambiente. Os testes nativos não simulam PPSSPP; compile e teste no emulador. Não assumimos que os outros 34 registros já possam carregar CHRs/SFX físicos. Nunca edite a ISO original.
