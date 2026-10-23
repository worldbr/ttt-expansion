@echo off
setlocal
if "%~1"=="" (
 echo USO: INSTALAR_PLUGIN_WINDOWS.bat "C:\CAMINHO\MEMSTICK"
 echo A pasta precisa ser a raiz do Memstick exibida em PPSSPP: File > Open Memstick Folder.
 pause
 exit /b 1
)
set "ROOT=%~f1"
set "SRC=%~dp0TTT_EXPANSION"
if not exist "%SRC%\ttt_expansion.prx" (
 echo PRX NAO ENCONTRADO em "%SRC%".
 echo Primeiro compile no GitHub Actions e extraia o artifact TTT-Expansion-PRX-REBUILD-V2000.
 pause
 exit /b 2
)
if not exist "%ROOT%\PSP" (
 echo Memstick nao reconhecido: falta a pasta PSP em "%ROOT%".
 pause
 exit /b 3
)
set "DST=%ROOT%\PSP\PLUGINS\TTT_EXPANSION"
if not exist "%DST%" mkdir "%DST%"
xcopy "%SRC%\*" "%DST%\" /E /I /Y >nul
if errorlevel 1 (echo ERRO AO COPIAR.&pause&exit /b 4)
echo OK: PRX E ASSETS COPIADOS PARA "%DST%"
echo Reinicie PPSSPP completamente. Nao carregue save state de versoes antigas.
pause
