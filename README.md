# TTT Expansion V0.9.8.2 — Teste da NOVA entrada 14098 do PACKFILE

**Alvo:** Dragon Ball Z: Tenkaichi Tag Team (ULUS10537), o mesmo EBOOT da pesquisa. **Experimental; teste só com backup da ISO e do PRX.**

## Objetivo específico

Esta versão mantém o **GPU Guard**, a interface, a ponte de Goku Early `ID 00`, a injeção dos quatro jogadores da V0.9.8 e os logs. Sua alteração principal, baseada na V0.9.8.1, é redirecionar `Goku Early ID 00, Color 1` para o **índice zero-based `0x3711` (14098 no Index Editor)**, em lugar da entrada 1194 (`0x4A9`).

```
Goku Early (ID 00), Color 1
       | hook dos recursos (resolvers)
       v
resource index = 0x3711 (14098), SOMENTE com PACKFILE count=0x3712
       | carregador de arquivos original do jogo
       v
registro 0x3711 -> ALIAS do CHR Goku Base original (indice 0x593)
```

**Primeiro teste: o registro 14098 é um alias do arquivo Goku Base original, sem inserir dados físicos extras.** Portanto a aparência é igual à do Goku Base; o foco é verificar se o carregador aceita **o índice 0x3711**, sem acessar a entrada 1194. O teste **não prova ainda** a carga de um novo arquivo físico, nem a expansão completa de limites do motor.

## Pré-requisitos obrigatórios

- **ISO/CÓPIA** em que o PACKFILE tem `PacK`, contagem **`0x3712`**, e o registro novo no offset `0x37120` do PACKFILE é o alias anteriormente preparado:
  ```
  6C 7C 61 4B 11 07 14 00 50 61 63 4B 00 00 00 00
  ```
  O registro aponta para o mesmo offset `0x10ED6000` e tamanho `0x143000` do CHR Goku Base (neste arquivo específico).
- **EBOOT já corrigido** para aceitar `0x3712` em vez de `0x3711` (conforme o teste anterior que permitiu iniciar a luta). **O PRX deste pacote NÃO altera essa checagem do EBOOT; ele é instalado depois que o jogo já iniciou.**
- Se ainda estiver usando PACKFILE `0x3711`, a V0.9.8.2 deverá **recusar o redirecionamento** com `error=0xFFFFFFF5` (`-11`). Use a V0.9.8/V0.9.8.1 em vez dela com a ISO original.
- Recomenda-se desativar outros cheats que alterem tabelas de personagens ou offsets de PACKFILE.

## Como testar com segurança

1. Guarde backups da V0.9.8 funcional e da ISO antes de iniciar.
2. Atualize o projeto do GitHub com os arquivos deste pacote e execute **Actions → Compilar PRX do menu**. O artifact se chama `TTT-Expansion-PRX-V0982`.
3. Instale o PRX no PPSSPP e **reinicie completamente o emulador**. Verifique primeiro que uma batalha normal funciona **sem abrir o Expansion**.
4. Reinicie, abra com **L + R + Quadrado** e confirme **1P → Goku Early → Color 1 → X**.
5. Feche com Círculo, escolha outro personagem no menu original, por exemplo Piccolo, e inicie a batalha.
6. Veja se entrou com Goku Base, consegue atacar e terminar a luta sem falhas. Depois confira `diagnostico.txt`.
7. Se funcionar para 1P, opcionalmente teste os outros slots; eles usam a mesma nova entrada. Para isolar problemas, não comece ativando os quatro ao mesmo tempo.

## Diagnóstico esperado

Exemplos, **não são saídas já verificadas no PPSSPP**:

```
[TTT V0.9.8.2] RESOURCE CONFIRMAR installed=0x00000001 error=0x00000000 ...
  size_14098=0x00143000 size_goku=0x00143000
  record_id=0x00003711 off_14098=0x10ED6000 off_goku=0x10ED6000
  pack_magic=0x4B636150 count=0x00003712

[TTT V0.9.8.2] RESOURCE MATCH count=0x00000001
  id=0x00000000 offset=0x00000000 old=0x00000593 new=0x00003711 path=0x00000001

[TTT V0.9.8.2] ACTOR INJECT STATUS ... hits=0x00000001 ...
```

Se `RESOURCE CONFIRMAR` mostrar `installed=0`, confira `error` e as colunas `record_id`, `off_14098`, `size_14098` e `count`:

| Erro decimal | Motivo do teste | Ação |
| --- | --- | --- |
| `-10` | assinatura `PacK` não reconhecida na RAM | Checar ISO correta e reiniciar |
| `-11` | contador RAM diferente de `0x3712` | Conferir cabeçalho e EBOOT modificado |
| `-12` | identificador do registro RAM diferente de `0x3711` | Compartilhar log para estudar formato do índice carregado |
| `-13` | tamanho inválido do novo registro | Conferir registro codificado |
| `-14` | offset da nova entrada não coincide com o CHR Goku Base | Usar o registro alias exato do teste |
| `-15` | tamanho do novo registro não coincide com Goku Base | Usar o registro alias exato do teste |

**Importante:** `RESOURCE MATCH new=0x3711` demonstra que nosso hook devolveu a nova posição. Se Goku aparece e a batalha roda, temos boa evidência do uso do índice adicional. Para provar fisicamente a leitura de um CHR diferente por essa posição precisaremos, num passo posterior, apontá-la para um modelo visualmente distinto e remover a validação de alias específico.

## Limitações

- O plugin não cria o registro do PACKFILE nem corrige o EBOOT; ambos devem estar previamente aplicados na **ISO de teste**.
- O registro novo alias não implica arquivo extra real gravado no fim do PACKFILE.
- Só **Goku Early ID 00 / Color 1** está associado a recurso experimental; Goku Mid `01` ainda é prévia.
- Mantenha backup e pare o teste se houver corrupção gráfica/travamento; restaure a V0.9.8 e a ISO original.
- A compilação completa com o PSPSDK e o teste real do PPSSPP **ainda precisam de validação do usuário**.

## Fontes principais

- `src/resource_hook.h`: define o novo índice `0x3711`.
- `src/resource_hook.c`: checa contagem, índice e alias; altera apenas o retorno dos cinco JALs de resolução.
- `src/main.c`: mantém injeção de 1P–4P e imprime `RESOURCE MATCH`, `off_14098` etc.
- `src/actor_inject.S`: mesmo hook MIPS de inicialização aprovado na V0.9.8.
- `tests/test_resource.c`: verificações nativas de limites e da posição `0x08B2A360`.

O conteúdo de `docs/TESTE_PACKFILE_3712_V0981.md` é preservado **como histórico da versão anterior**, que ainda usava a entrada 1194. Para esta versão, siga este README ou `docs/TESTE_NOVA_ENTRADA_14098_V0982.md`.
