# TTT Expansion PRX V0.9.3 — Diagnóstico do primeiro CHR independente

A V0.9.2 carregou a batalha normalmente com Goku Early (`ID 00`) e instalou o hook de recursos; o log mostrou `redirects=1`, mas os campos `last_id`, `old` e `new` foram sobrescritos por uma solicitação posterior do ID `0x12`. **Esta versão mantém exatamente o mesmo redirecionamento e corrige o diagnóstico, não adiciona um Mod Loader externo.**

## O que muda

- Preserva **GPU Guard**, menu V0.8.1, retratos, seleção 1P–4P e o bloqueio de controles.
- Preserva Goku Early `ID 00` + ponte temporária Videl forma 2.
- Preserva a regra **ID 00 + Color 1 → índice PACKFILE 0x4A9 (entrada 1194 no Index Editor)**, sem mudar o Goku `ID 03`.
- Registra separadamente o último **redirecionamento real**, para que as solicitações posteriores de outros personagens não sobrescrevam `id/offset/old/new` de interesse.
- Novo registro `RESOURCE MATCH`: `count`, `id`, `offset`, `old`, `new`, `path` (1 = resolver checado, 2 = não checado).
- Não realiza I/O no interceptor; logs continuam vindo da thread principal.

## Compilar e testar

1. Faça backup do PRX V0.9.2. Use sua ISO atual contendo o CHR do Goku na entrada **1194**, e desative cheats antigos de expansão. Não substitua outros recursos.
2. Atualize seu repositório GitHub com os arquivos do ZIP, faça commit e execute **Actions → Compilar PRX do menu**. Artifact esperado: `TTT-Expansion-PRX-V093`.
3. Instale o PRX novo e reinicie completamente o PPSSPP.
4. Teste rapidamente o Goku original `03` em batalha para garantir que os gráficos continuam normais.
5. Abra o TTT Expansion por `L+R+Quadrado`; selecione `1P → Goku Early → Color 1`, confirme `X`. Feche com Círculo.
6. Selecione a **segunda forma experimental da Videl** no menu original, entre em batalha, e depois confira `diagnostico.txt`.

## Resultado esperado no log

```text
[TTT V0.9.3] RESOURCE MATCH count=0x00000001 id=0x00000000 offset=0x00000000 old=0x00000593 new=0x000004A9 path=0x00000001
```

A linha acima é **exemplo**, não resultado real. `old` e `path` poderão variar. `RESOURCE STATUS` preserva o formato e os contadores anteriores. O `RESOURCE MATCH` informa a chamada em que ocorreu o redirecionamento; mesmo confirmando `new=0x4A9`, ainda não comprova, sozinho, a leitura final do arquivo ou o uso dos dados pelo renderizador. Como a entrada 1194 contém uma cópia visualmente idêntica, o modelo deve permanecer igual.

Se `count=0`, a condição do hook não foi atendida. Se houver travamentos, gráficos corrompidos ou erros, feche o PPSSPP e restaure a V0.9.2. Nunca desarme a ponte no meio da batalha.

### Próxima etapa

Depois de validar `RESOURCE MATCH`, comprovar a leitura efetiva da entrada `0x4A9` e testar uma Color visivelmente diferente em um arquivo separado, antes de criar o carregamento de PCK1 por pasta externa.
