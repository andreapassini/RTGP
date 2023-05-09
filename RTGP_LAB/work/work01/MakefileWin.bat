@echo off
IF EXIST "C:\Program Files (x86)\Microsoft Visual Studio\BuildTools2022\VC\Auxiliary\Build\vcvarsall.bat" (
    call "C:\Program Files (x86)\Microsoft Visual Studio\BuildTools2022\VC\Auxiliary\Build\vcvarsall.bat" x64
) ELSE (
    call "C:\Program Files (x86)\Microsoft Visual Studio\BuildTools2022\VC\Auxiliary\Build\vcvarsall.bat" x64
)

if [%1%]==[] (
  nmake /f MakefileWin all
) else (
  nmake /f MakefileWin clean
)


