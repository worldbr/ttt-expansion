# Encontrar quem escreve o ID de personagem — PPSSPP Debugger

**Objetivo:** localizar o endereço **PC da instrução MIPS** que escreve o ID do 1P para poder estudar a cadeia de preparação **ANTES** da criação do personagem. Não é necessário modificar o valor da RAM para isso.

## Endereços de observação confirmados

| Jogador | ID | Color |
| --- | --- | --- |
| 1P | `0x08B48AE8` | `0x08B48AF0` |
| 2P | `0x08B48BC0` | `0x08B48BC8` |
| 3P | `0x08B48CB8` | `0x08B48CC0` |
| 4P | `0x08B48D90` | `0x08B48D98` |

## Procedimento

1. Antes de selecionar a luta, abra o **Debugger** do PPSSPP (disponível em builds de desktop; a localização do menu varia entre versões).
2. Na janela de **Memory View/Breakpoints**, tente criar um **breakpoint de escrita de memória / memory write watchpoint**, comprimento **4 bytes**, para `0x08B48AE8`. Se o PPSSPP exigir formato sem prefixo, use `08B48AE8`.
3. **Não crie breakpoint de execução** no endereço de dados e não modifique o conteúdo. O tipo desejado é **escrita de RAM**.
4. Volte ao jogo, escolha um personagem 1P **diferente de Goku Early** no menu original, e entre na batalha.
5. Quando o emulador interromper a execução, anote o valor de **PC / instruction address**, a instrução MIPS no disassembly, registros de base/offset e o valor anterior/novo em `0x08B48AE8`. Guarde screenshot da janela.
6. Se pausar **muitas vezes**, registre a **primeira escrita** a partir da confirmação da batalha e uma das últimas antes de a luta começar. O primeiro breakpoint pode ocorrer por limpar ou inicializar a estrutura, não necessariamente pela escolha do personagem.
7. Desative o breakpoint antes de retomar os testes normais. Se o PPSSPP usado não oferecer watchpoints de escrita, pule esta parte e envie somente `ACTOR CHANGE`.

## Por que o PC é importante

Nosso hook de resolução CHR calcula o **número do recurso**, mas não sabe com segurança a qual jogador corresponde a chamada. O `ACTOR RAW` mostra o resultado **depois** que o jogo já definiu o ID. Precisamos rastrear o código que grava o campo e, a partir dele, descobrir de onde o valor foi copiado. Só então avaliar se é seguro trocar os dados antes de o motor carregar CHR/SFX, atributos e animações.

> Breakpoints podem interromper em funções de inicialização/limpeza e não necessariamente na seleção. É preciso examinar a sequência e a chamada de origem antes de criar um patch de escrita.
