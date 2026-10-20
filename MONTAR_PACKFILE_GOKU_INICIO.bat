@echo off
setlocal
if "%~1"=="" (
 echo Modo de usar: MONTAR_PACKFILE_GOKU_INICIO.bat "C:\PASTA\PACKFILE.BIN"
 echo Passe o PACKFILE.BIN COMPLETO, nao o cabecalho de 224 KB.
 pause
 exit /b 1
)
set "SRC=%~f1"
set "DST=%~dp0PACKFILE_GOKU_INICIO_C1_3734.BIN"
if exist "%DST%" (
 echo ARQUIVO DESTINO JA EXISTE: %DST%
 echo Mova/remova o resultado ANTES de executar de novo.
 pause
 exit /b 1
)
where py >nul 2>&1
if not errorlevel 1 (
 py -3 "%~dp0tools\expandir_packfile_7_gokus.py" --input "%SRC%" --output "%DST%" --asset-dir "%~dp0EXPANSAO_7_GOKUS\assets"
) else (
 python "%~dp0tools\expandir_packfile_7_gokus.py" --input "%SRC%" --output "%DST%" --asset-dir "%~dp0EXPANSAO_7_GOKUS\assets"
)
if errorlevel 1 (
 echo ERRO: nao foi possivel montar o PACKFILE. Verifique a mensagem acima.
 pause
 exit /b 1
)
echo PACKFILE NOVO CRIADO EM: %DST%
echo IMPORTANTE: nao sobrescreva a ISO no HxD. A ISO precisa ser reconstruida
 echo com o novo tamanho de PACKFILE.BIN no sistema ISO9660.
pause
