# TTT Expansion V0.9.8.8 — Proteção da lista gráfica

## Motivo da correção
Na V0.9.8.7, o usuário observou menu aberto que parava de desenhar depois de confirmar um jogador e passar para outro.
O diagnóstico mostrou simultaneamente:

```
MENU WATCHDOG open=0x00000001 draw_enabled=0x00000000 ge_failed=0x00000000
HOOK installed=0x571ED6FF calls=0x551ED773
```

O valor `0x571ED6FF` é compatível com um comando da GPU que usa a cor do menu `0x1ED6FF`, não com uma variável booleana. Isso é evidência forte de gravação de comandos GU fora da lista usada pelo menu. A lista tinha 32768 palavras (128 KiB), sem orçamento de comandos.

## Mudanças na V0.9.8.8
- 131072 palavras / 512 KiB na lista GU (4x o tamanho anterior).
- Orçamento preventivo: até 4096 retângulos/portraits por quadro; os excedentes são ignorados para evitar corrupção.
- Canários de 128 palavras antes/depois da lista para registrar `guard_fault` se houver transbordamento. Se corrompido, o GPU Guard impede a continuação dos desenhos e restaura o contexto da GPU.
- Diagnóstico `GE GUARD` acrescenta `primitives`, `peak`, `skipped`, `guard_fault`, `list_bytes`.
- Diagnóstico `MENU WATCHDOG` acrescenta `primitives`, `skipped`, `canary`.
- Mantidos IDs 00, 01, 02, todas as 4 Colors e o mecanismo de injeção 1P–4P da V0.9.8.7.

## Como testar
1. NÃO modifique PACKFILE/ISO/EBOOT. Use a ISO com o índice expandido `0x3734` que já funciona.
2. Atualize a raiz do repositório com este projeto ou aplique o ZIP de atualização *sobre a V0.9.8.7*.
3. Compile no GitHub Actions, artefato esperado `TTT-Expansion-PRX-V0988`.
4. Reinicie totalmente o PPSSPP (não carregue save-state com o PRX antigo).
5. Abra com L+R+Quadrado. Confirme Goku Início para 1P, passe para 2P com R, confirme Goku Mid, passe para 3P e 4P, testando troca de personagem e Color.
6. Verifique se o menu continua visível, fecha com Círculo e reabre normalmente.
7. Envie trecho novo do `diagnostico.txt` caso haja erro.

## Interpretação dos logs
- `draw_enabled=1`, `gpu_rc=0`, `guard_fault=0`, `skipped=0`: resultado ideal.
- `skipped>0`: atingido o orçamento por quadro; houve omissão de desenhos, mas o orçamento impediu continuar além do limite.
- `guard_fault=1`: possível overflow mesmo com as defesas; NÃO ignore, encaminhe o diagnóstico.
- `gpu_rc<0`: erro vindo da GPU/GU; o renderizador para por proteção.

## Validação
Testes C de orçamento/canários e suíte nativa do projeto passaram no PC. **Sem confirmação de compilação PSPSDK nem de PPSSPP até que o usuário teste.**
