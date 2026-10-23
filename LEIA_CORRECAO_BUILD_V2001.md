# Corrigido: erro 127 no GitHub Actions — DLC Rebuild 2.0.1

## Diagnóstico
O workflow do Rebuild 2.0 chamava `python3 tools/inspecionar_packfile.py --help`
no container oficial `pspdev/pspdev:v20261001`. A imagem e baseada em Alpine,
mas nao instala Python 3 no estagio final (veja o Dockerfile oficial do PSPDEV).
Isso e uma explicacao muito provavel para `Process completed with exit code 127`.
Sem o log completo do run nao e possivel identificar com certeza absoluta qual
comando faltou; a V2.0.1 registra todos os comandos ate o ponto de falha.

## O que foi corrigido
- Verifica a existencia de `python3` e instala automaticamente com `apk add --no-cache python3` se estiver ausente.
- Cria `build.log` ANTES da verificacao de ferramentas.
- Redireciona toda a saida do processo de compilacao para `build.log` e para o console do GitHub.
- Publica o arquivo de log mesmo que o build falhe.
- Valida tamanho e presenca das texturas antes de compilar.
- Mantem o codigo do PRX, a ISO, o PACKFILE, os personagens, 12 Colors e desafios inalterados.

## Instrucoes
1. Extraia este ZIP de atualizacao na **raiz** do repositorio `worldbr/expansionttt2`,
   preservando o caminho `.github/workflows/compilar-prx.yml`. Substitua o arquivo antigo.
   Alternativamente use o ZIP completo da V2.0.1.
2. Confirme e envie o commit para o GitHub. Execute o workflow `TTT DLC Rebuild V2 - PRX`.
3. Se funcionar, baixe `TTT-Expansion-PRX-REBUILD-V2001`.
4. Se falhar, baixe `TTT-Expansion-Build-Log-V2001` e envie o arquivo `build.log`.
   O erro verdadeiro agora deve aparecer dentro dele.

O aviso do GitHub sobre Node.js 20/actions/upload-artifact@v4 nao e a causa
imediata do codigo 127 e pode ser ignorado enquanto o build e corrigido.

**Nao ha PRX precompilado neste ZIP.** O PSPSDK real nao esta instalado neste
ambiente; os testes locais usam GCC do computador, nao substituem o build no GitHub.
