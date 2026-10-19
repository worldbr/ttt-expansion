# EXPANSAO 7 GOKUS — TTT EXPANSION V0.9.8.4 (EXPERIMENTAL)

## O que está entregue

**São sete formas, não seis.** Goku Início, Goku Mid, Goku Mid SSJ, Goku GT, Goku GT SSJ, Goku GT SSJ3 e Goku GT SSJ4.

Foram RESERVADAS 35 novas entradas no índice: 4 CHRs (Color 1–4) e 1 SFX por forma. A numeração vai de **14098 a 14132** (base zero `0x3711..0x3733`). A contagem do PACKFILE passa para `0x3734` (14.132). Restam 203 registros físicos até 0x38000.

O arquivo `CABECALHO_35_SLOTS_0x3734.bin` é uma **imagem pronta dos primeiros 0x38000 bytes do PACKFILE**, criada a partir do cabeçalho experimental `0x3712` que já funcionou no teste anterior. Não contém modelos ou SFX novos; todos os CHRs reservados apontam por ora para o CHR original do Goku End/Base (resource ID `0x593`) e os SFX reservados apontam para um SFX original do Goku (resource ID `0x597`). Essas referências são temporárias, justamente para testar a estrutura SEM aumentar o PACKFILE.

O PRX V0.9.8.4 foi adaptado para esperar `0x3734`, com menu das sete formas; **só Goku Early (ID 00), Color 1, possui bridge/hook/injeção de batalha comprovados.** Os outros nomes/colors são PREVIA/RESERVA, sem prometer personagens jogáveis ou SFX associados. Os IDs externos do Goku Mid e Mid SSJ são 01 e 02 (conhecidos); os IDs GT ainda exigem identificação. Os recursos físicos dos sete personagens NÃO foram fornecidos, portanto este pacote não cria os modelos que faltam.

## Teste A: expansão do índice para 35 reservas, SEM anexar assets (recomendado primeiro)

1. Faça backup da ISO, PACKFILE.BIN, EBOOT e PRX atual.
2. Abra uma **cópia** do PACKFILE.BIN no HxD. Abra também `CABECALHO_35_SLOTS_0x3734.bin`. Copie seus 0x38000 bytes e SOBRESCREVA os bytes `0x00000..0x37FFF` do PACKFILE.BIN copiado (modo sobrescrever, SEM inserir bytes). O tamanho do PACKFILE.BIN não muda.
3. Modifique a cópia do EBOOT.OLD **ELF específico** de ULUS10537: em offset de arquivo **0x2CBD4**, o opcode de 4 bytes era `11 37 1E 34` na versão original e `12 37 1E 34` na versão experimental anterior. Alterar para **`34 37 1E 34`** (instrução RAM `ori $fp,$zero,0x3734` em 0x0882FBFC). Alternativamente, use `tools/patch_eboot_contagem.py`, que valida o opcode e gera uma nova cópia. **O offset ELF não é o offset na ISO.** Não aplique cegamente a um EBOOT de outra versão/região.
4. Reinsira ambos na **cópia** da ISO usando procedimento conhecido de reconstrução; se o editor reescrever o PACKFILE depois, reaplique o cabeçalho preparado ao final. Sem alterar o comprimento original do PACKFILE, substituição in-place é viável, mas cuidado com deslocamento relativo do PACKFILE na ISO.
5. Compile o plugin V0.9.8.4 com GitHub Actions. Instale novo PRX e reinicie totalmente o PPSSPP.
6. Primeiro teste a abertura e uma batalha normal. Depois ative Goku Early Color1 nos quatro jogadores pelo Expansion e comece uma nova luta.
7. No diagnóstico, procure `count=0x00003734`, `record_id=0x00003711`, `RESOURCE MATCH ... new=0x00003711` e os `ACTOR INJECT SLOT ... hits` dos quatro jogadores. Se aparecerem os quatro Gokus End, isso é ESPERADO: as 28 entradas CHR são aliases dos arquivos antigos neste primeiro teste.

**Se tela preta:** restaure backup; pode existir outra verificação de quantidade no EBOOT. Não conclua que as 35 entradas foram aceitas até testar no PPSSPP.

## Teste B: adicionar CHRs e SFX FÍSICOS quando você enviar os arquivos

Estrutura de pasta de `--asset-dir`:

```
assets/
  GOKU_EARLY/C1.PCK1, C2.PCK1, C3.PCK1, C4.PCK1, SFX.PCK1
  GOKU_MID/C1.PCK1, C2.PCK1, C3.PCK1, C4.PCK1, SFX.PCK1
  GOKU_MID_SSJ/C1.PCK1, C2.PCK1, C3.PCK1, C4.PCK1, SFX.PCK1
  GOKU_GT/C1.PCK1, C2.PCK1, C3.PCK1, C4.PCK1, SFX.PCK1
  GOKU_GT_SSJ/C1.PCK1, C2.PCK1, C3.PCK1, C4.PCK1, SFX.PCK1
  GOKU_GT_SSJ3/C1.PCK1, C2.PCK1, C3.PCK1, C4.PCK1, SFX.PCK1
  GOKU_GT_SSJ4/C1.PCK1, C2.PCK1, C3.PCK1, C4.PCK1, SFX.PCK1
```

Exemplo Windows PowerShell, com Python instalado:

```
py -3 tools\expandir_packfile_7_gokus.py --input "D:\TTT\PACKFILE.BIN" --output "D:\TTT\PACKFILE_7_GOKUS.BIN" --asset-dir "D:\TTT\assets"
```

- Com `--asset-dir` ausente, todas as entradas são aliases e o BIN final mantém o tamanho antigo.
- Com `--asset-dir` presente, somente arquivos realmente presentes são ANEXADOS ao final do BIN com alinhamento 0x800; os demais slots ficam com alias. A ferramenta produz relatório `.json` e `.csv` com cada índice, offset e tamanho.
- `--require-all-assets` recusa criar o arquivo se algum dos 35 CHR/SFX estiver ausente.
- O empacotador usa o algoritmo de codificação do Editor de Índice recuperado do EXE compartilhado. Confere ida/volta de todos os registros.
- **IMPORTANTE: anexar arquivos aumenta o comprimento de PACKFILE.BIN.** Substituir bytes sem atualizar o tamanho de `PACKFILE.BIN` nos metadados ISO9660 pode falhar, mesmo se o cabeçalho estiver correto. Recrie a ISO com atualização de extensão/tamanho, não apenas sobreponha bytes. O empacotador edita um BIN isolado, não uma ISO.
- Modelos/SFX inválidos ou de jogo diferente podem travar o loader; estes assets são responsabilidade da etapa de adaptação de arquivos.

## Notas de segurança e limitações

- Nunca edite os arquivos originais sem backup.
- Plugin PRX não compila neste pacote local sem PSPSDK; código e testes nativos foram verificados, e a compilação PSP/PPSSPP precisa ser feita por você via GitHub Actions.
- Os slots CHR e SFX são endereços/arquivos organizados, não registro automático de sete personagens jogáveis. O registro de formas, fórmulas de recurso, IDs externos, SFX e Color no EBOOT ainda precisa ser ampliado por etapas.
- O cabeçalho `0x3734` e EBOOT proposto ainda NÃO foram testados juntos no PPSSPP.
