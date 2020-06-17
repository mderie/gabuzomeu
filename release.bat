@echo off

echo "Are you sure to start the release process ? If not just break"
pause

if [%1] == [] goto error

xcopy x64\Release\gabuzomeu.exe material\

del /s *.tlog 
rem del ...

rem git add .
rem git commit -m %1
rem git push https://github.com/mderie/gabuzomeu
goto end

:error

echo "Please provide a label for the commit"
goto eof

:end

echo "That's all folks !"

:eof
