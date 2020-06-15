@echo off
del *.tlog
del ...
git add .
git commit -m %1
git push https://github.com/mderie/gabuzomeu