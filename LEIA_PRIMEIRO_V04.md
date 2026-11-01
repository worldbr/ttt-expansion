# TTT Expansion — 47 IDs + nomes individuais (V0.4 EXPERIMENTAL)

**Jogo:** Dragon Ball Z: Tenkaichi Tag Team, ULUS10537, somente PPSSPP. 
**Status:** EBOOT/HEADER validados estaticamente; 47 novos IDs NÃO foram todos testados em batalha. A V0.3 com IDs 00,01,02 já foi testada pelo usuário, mas a V0.4 ainda precisa de teste no PPSSPP.

## O que está no pacote

- `EBOOT.V04_47IDS_EXPERIMENTAL.ELF`: 3.747.908 bytes, mesmo tamanho do ELF V0.3. Atualiza dois resolvedores internos e as tabelas de IDs/grupos; contador PACKFILE=0x37FC.
- `PACKFILE_CABECALHO_47_ALIAS_GOKU.BIN`: **somente os 224 KiB do CABEÇALHO** que o usuário forneceu; NÃO é um PACKFILE completo. Contagem 0x37FC.
- `MAPA_47_PERSONAGENS.csv`: os 47 slots, IDs de combate, índices CHR/Colors/SFX.
- `TTT_EXPANSION/characters.txt`: nomes editáveis individualmente, `01=...` até `47=...`; o PRX lê/recarrega ao abrir o menu.
- `tools/aplicar_cabecalho_47_v04.py`: cria uma cópia do PACKFILE completo mantendo todos os dados físicos, mas trocando o cabeçalho.
- Pacote de atualização GitHub do PRX: aceita todos os 47 IDs **apenas** ao reconhecer o EBOOT V0.4 e contagem 0x37FC. V0.3 continua permitida para os três primeiros.

## Decisões técnicas

A lista enviada pelo usuário ocupa 70 IDs. Além desses, **0x46 e 0x47 foram mantidos reservados**, pois surgiram como aliases de Vegeta em outra investigação; não foram atribuídos aos 47 extras. Os 47 IDs em ordem crescente estão em `MAPA_47_PERSONAGENS.csv`.

Para evitar colisão com o comando Aleatório do menu original (`0x46`), a V0.4 usa os valores 0x80..0xAE como **marcadores internos de índice**, e não como battle IDs. O PRX usa os battle IDs do mapa, não os marcadores.

Cada slot recebe 5 índices: C1,C2,C3,C4 e SFX; `0x3711..0x37FB`, quantidade 0x37FC. Os 35 índices existentes (`0x3711..0x3733`) permanecem **inalterados**. Os 200 restantes (`0x3734..0x37FB`) passam a ser aliases de modelos e áudio do Goku End original (`0x593..0x597`), apontando para dados físicos já existentes no PACKFILE. Há **três registros de Color 3** que precisam usar Goku C1 como fallback devido a um detalhe do codificador de tamanho obfuscado do índice. Nada disso cria 44 modelos novos: os personagens ainda terão aparência do Goku até seus arquivos serem substituídos.

Além das tabelas, o EBOOT V0.4 aproveita os **mesmos limites físicos das duas funções** `0x089C6CA0` e `0x089C6D60`; nenhum segmento ELF novo foi acrescentado. A tabela de grupos `0x08A69720` devolve o grupo 0 do Goku para os 47 IDs, evitando consultas ao grupo `FF` em sistemas auxiliares. Isso é um fallback; pode não abranger todos os sistemas internos.

## Instalação cautelosa

1. Salve backups da ISO expandida V0.3 que funciona, do EBOOT e do PACKFILE completo.
2. Para aplicar o cabeçalho ao PACKFILE completo, execute no Windows com Python:

   ```bat
   py tools\aplicar_cabecalho_47_v04.py --packfile "D:\PACKFILE.BIN" --header "PACKFILE_CABECALHO_47_ALIAS_GOKU.BIN" --output "D:\PACKFILE_V04_47.BIN"
   ```

   O script exige que o PACKFILE de entrada tenha exatamente o cabeçalho original fornecido, contagem `0x3734` e dados físicos suficientes; não sobrescreve o original. Se faltar algum dado, ele **recusa** o processo. Se a ISO tiver um PACKFILE diferente, não force a instalação.
3. Reconstrua uma **ISO de teste**, colocando o PACKFILE completo de saída no lugar do PACKFILE anterior, atualizando o tamanho corretamente se necessário. Os ponteiros novos não aumentam os dados físicos.
4. Substitua apenas o ELF principal EBOOT.OLD pelo `EBOOT.V04_47IDS_EXPERIMENTAL.ELF` usando o GameUtility, como você fez na V0.3. Não sobrescreva o BOOT.BIN.
5. Compile o PRX com o projeto V0.4 pelo workflow GitHub Actions e instale o PRX junto a `TTT_EXPANSION/characters.txt`. O diretório deverá ser `ms0:/PSP/PLUGINS/TTT_EXPANSION/characters.txt`.
6. Desative versões antigas do PRX para não haver dois plugins tentando escrever nos IDs.
7. No PPSSPP, teste primeiro uma batalha ORIGINAL com Goku (Fim) Base, depois os três Gokus da V0.3, depois teste o índice do novo slot 8 **SEM continuar a batalha**, examinando apenas o retorno do resolvedor.

### Verificação de um novo slot no PPSSPP

- Slot 08: **Battle ID `0x0B`**, Color 1 (variante 0); retorno esperado de `0x089C6CA0` ou `0x089C6D60`: **`0x3734`**.
- Slot 47: battle ID `0x73`, Color 1 (variante 0); retorno esperado: **`0x37F7`**.
- Para original Goku (Fim) Base, battle ID `0x03`, variante 0: retorno **`0x0593`**, sem mudança.

No depurador, restaure um save state após testes alterando registradores manualmente; não continue uma batalha com parâmetros de chamada manipulados. Se o jogo travar, volte à V0.3 original e envie o endereço da exceção / `PC`, `RA`, `A0`, `A1` e o log.

## Editando o nome dos 47 personagens

Abra `TTT_EXPANSION/characters.txt` num editor de texto e troque individualmente qualquer valor:

```ini
01=Goku (Inicio)
02=Goku (Meio)
03=Goku (Meio) SSJ
04=Meu Novo Personagem
...
47=Meu Personagem 47
```

Use nomes com até ~25 caracteres visíveis. A fonte desenhada atualmente pelo plugin é ASCII (caracteres acentuados UTF-8 não são reproduzidos corretamente); use `Inicio` em vez de `Início` até instalarmos um atlas com acentos. Ao **fechar e reabrir o menu**, a atualização é carregada. Os nomes não são escritos nos arquivos de texto originais do jogo; aparecem **somente no menu PRX**.

## Limitações honestas

- Header e EBOOT V0.4 são experimentais. Os 44 novos battle IDs ainda precisam de testes em tempo real para confirmar que outros caminhos da engine não tratam algum deles como reservado.
- Os 47 nomes estão disponíveis no plugin, mas o nome nativo em outras telas será o do Goku enquanto utilizarmos a tabela de grupos com fallback 0.
- O arquivo recebido contém só cabeçalho; não é possível montar um PACKFILE inteiro e fiel sem o PACKFILE completo.
- O PRX não foi compilado em um ambiente PSPDEV nesta sessão; o workflow GitHub prepara o PRX.
