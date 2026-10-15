@echo off
setlocal
cd /d "%~dp0"
set QT_QPA_PLATFORM=windows
set QT_PLUGIN_PATH=%~dp0
set QT_QML_IMPORT_PATH=%~dp0\qml
set QT_DEBUG_PLUGINS=0
start "Projeto Glossata" "%~dp0\projeto_glossata.exe"
