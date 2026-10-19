# TTT Expansion PRX V0.9.5 — Pré-carregamento (diagnóstico por jogador)

## Ideia adotada

O usuário escolhe **qualquer** personagem em qualquer posição do menu ORIGINAL apenas para chegar à batalha. O menu **TTT Expansion** armazena a escolha independente dos slots 1P–4P. A substituição de ID, Color e recursos deverá acontecer **ANTES** do carregamento da batalha, não depois de ela estar pronta.

**IMPORTANTE: V0.9.5 É UMA ETAPA DE INSTRUMENTAÇÃO, NÃO IMPLEMENTA A TROCA AUTOMÁTICA AINDA.** O mapeamento 'recurso solicitado → jogador' ainda é desconhecido; alterar o ID nos hooks atuais pode trocar o adversário por engano. A V0.9.5 coleta informações para encontrar o ponto correto, com segurança.

## Recursos que continuam funcionando

- GPU Guard da V0.8.2 (fix do cenário preto)
- Interface visual V0.8.1, retratos RGBA e controles L+R+Quadrado
- Escolhas separadas 1P,2P,3P,4P no menu, Color1–4
- ID00 + CHR da entrada PACKFILE 1194 para Goku Early Color1 (experimental)
- Quinta forma do Goku original como portador temporário; Videl não modificada, herdado da V0.9.4 (ainda não confirmado no PPSSPP)

## Melhorias V0.9.5

1. Módulo `slot_plan`: grava a escolha do usuário separadamente em 1P,2P,3P,4P, com validação e número de confirmação. Goku Mid aparece no plano, mas sua identidade interna ainda não foi integrada.
2. **Cinco interceptores distintos**, um por callsite conhecido de cálculo de recursos. `LOAD TRACE` mostra `site=0..4`, ID externo solicitado e recurso devolvido. Atenção: `site` **NÃO é jogador**; representa a função que pediu recurso.
3. Histórico circular de até 64 eventos, sem I/O nem alocação dentro do hook; worker registra amostras durante a execução.
4. `ACTOR RAW`: observação em quatro endereços de jogador em memória de batalha fornecidos pelo usuário (IDs e Colors brutos de 32 bits); **nenhuma gravação nesses endereços**. Os dados podem conter lixo ANTES da batalha. Usá-los com os eventos `LOAD TRACE` para investigar as fases de carregamento.
5. `PLAN CONFIRMAR` e `PLAN RESET` registram todas as intenções selecionadas. `applied=0` até descobrirmos o hook confiável.
6. Verificação adicional de Videl: os campos `Videl2` e `VidelEnd` continuam no diagnóstico como leitura de segurança; **o código não modifica a Videl**.

## Passo a passo (para quando voltar)

1. Faça backup do PRX V0.9.3/0.9.4. Compile V0.9.5 no workflow do GitHub e instale o PRX.
2. Reinicie PPSSPP. No TTT Expansion, selecione **Goku Early Color 1 em 1P** e aperte X. Para 2P, 3P, 4P, use L/R e confirme as escolhas (elas serão **registradas**, não impostas na batalha).
3. Feche o menu e escolha **qualquer personagem original** para 1P, como você propôs. Execute uma batalha; para o ID00 testado, se quiser carregá-lo NESTA versão, use a quinta forma do Goku em vez de um slot arbitrário.
4. Envie `PLAN CONFIRMAR`, `LOAD TRACE`, `ACTOR RAW`, `RESOURCE MATCH`, e diga quem você escolheu no jogo original em 1P e 2P.
5. Não tente usar os endereços ACTOR RAW como endereço pré-batalha. O objetivo do log é encontrar a relação entre seleção e carregamento.

## Limitações

A V0.9.5 **não** garante que quinto Goku aparecerá, nem substitui diretamente o ID de 1P–4P. Só será seguro implementar essa ação depois de rastrear o estado pré-carregamento e saber distinguir cada jogador. Não há suporte de CHR externo nesta linha: o recurso 1194 ainda vem da ISO. Se houver travamento, retorne à V0.9.3 e reinicie o PPSSPP.
