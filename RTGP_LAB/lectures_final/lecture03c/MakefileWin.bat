@echo off
IF EXIST "D:\Programmi\Microsoft Visual Studio\BuildTools2022\VC\Auxiliary\Build\vcvarsall.bat" (
    call "D:\Programmi\Microsoft Visual Studio\BuildTools2022\VC\Auxiliary\Build\vcvarsall.bat" x64
) ELSE (
    call "D:\Programmi\Microsoft Visual Studio\BuildTools2022\VC\Auxiliary\Build\vcvarsall.bat" x64
)

if [%1%]==[] (
  nmake /f MakefileWin all
) else (
  nmake /f MakefileWin clean
)