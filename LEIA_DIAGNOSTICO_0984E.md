# V0.9.8.4E — Diagnóstico por etapas (PACKFILE 0x3734)

**Não é uma versão para jogar normalmente.** Isola a falha de execução que ocorre logo depois de selecionar o Goku Início. Preserve backup da ISO e do PRX 0.9.8.3.

## Requisitos

Use a **mesma ISO experimental com PACKFILE count `0x3734` e EBOOT ajustado**, que abriu na V0.9.8.4D. Não altere EBOOT/PACKFILE para esses testes. Compile o PRX pelo GitHub Actions. Remova PRXs anteriores duplicados e reinicie completamente PPSSPP a cada modo.

## Como testar — ordem obrigatória

Abra o menu com L+R+Quadrado. Selecione **Goku Início, Color 1**, mas mude **o jogador**, usando R. A numeração do jogador define o MODO de diagnóstico neste build (não a escolha real da posição de batalha):

| Jogador mostrado | Modo | Alterações instaladas ao pressionar X |
|---|---|---|
| **1P** | Controle | **Nenhum hook de personagem**. Apenas confirma a interface. |
| **2P** | Recursos | Hook do resource resolver somente; não ativa o ID 00. |
| **3P** | Bridge | Hook de recurso + ponte ID 00; sem injeção de jogador. |
| **4P** | Completo | Recursos + bridge + injeção do ID 00 em 4P. |

**Para cada modo:** reinicie o PPSSPP por completo, abra o menu, selecione aquele jogador, pressione **X apenas uma vez**, espere uns 5 segundos. Registre se o jogo travou imediatamente, se o menu continua aparecendo e a última linha de `diagnostico.txt`. **Não feche o menu nem inicie uma luta nos modos 2P ou 3P**; são estados parciais intencionalmente experimentais. Se algum modo trava, não continue naquele modo; reinicie e teste o anterior.

Os marcadores úteis são `DIAG MODO_*`, `ETAPA_...`, `DIAG_APOS_GPU_MENU`, `DIAG_POS_X_FRAME`. Ver `ETAPA_4` apenas significa que o patch foi instalado, não que seja seguro na execução seguinte.

## Interpretação

- Se **1P** travar: não depende dos hooks; investigar interface, gráfico, memória ou conflito de plugins.
- Se 1P não travar, mas **2P** travar: investigar resource hook e limites do PACKFILE.
- Se 2P não travar, mas **3P** travar: investigar bridge / alteração das tabelas de ID.
- Se 3P não travar, mas **4P** travar: investigar trampoline de injeção e seu contato com as mudanças do carregador.
- Se todos passarem: o crash pode depender de tempo, batalha ou outro estado. Relate isso também.

### Hipótese ainda não comprovada

A tabela ampliada de `0x3712` para `0x3734` escreve 34 registros extras na RAM. O arquivo reserva `0x38000` bytes, mas não está comprovado que o programa tenha reservado o mesmo número de bytes para a tabela em memória. Um limite ou dado vizinho pode ser atingido. Não atribua o crash a essa hipótese sem os testes.

**Não há arquivos CHR/SFX novos neste pacote.**
