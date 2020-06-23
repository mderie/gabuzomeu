@echo off

echo "Are you sure to start the release process ? If not just break"
pause

rem if [%1] == [] goto error

xcopy /y x64\Release\gabuzomeu.exe material\

rem make a real clean !
del /s /q *.tlog 
del /s /q CharToCharGenerator\x64
del /s /q DataAsCodeGenerator\x64
del /s /q NumberGenerator\x64
del /s /q PhraseGenerator\x64
del /s /q RuleOfThree\x64
del /s /q SpaceGenerator\x64
del /s /q XXLengthGenerator\x64

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
