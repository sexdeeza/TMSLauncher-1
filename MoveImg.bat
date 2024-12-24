@echo off
setlocal enabledelayedexpansion
set "basefolder=Base.wz"
if not exist "%basefolder%" ( 
    echo Can't find "%basefolder%"
    pause
    exit /b 
)

echo Move img from Base folder...
pushd "%basefolder%"
move *.img ..
popd

echo Rename folder...
for /d %%f in (*.wz) do (
    set foldername=%%~nf
    ren "%%f" "!foldername!"
)

echo Delete Base folder...
rmdir /s /q "%basefolder%"

echo Done!

pause
