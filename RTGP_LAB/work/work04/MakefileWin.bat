@echo off
IF EXIST "D:\Programmi\Microsoft Visual Studio\BuildTools2022\VC\Tools\MSVC\14.35.32215\bin\Hostx64\x64\cl.exe" (
    call "D:\Programmi\Microsoft Visual Studio\BuildTools2022\VC\Tools\MSVC\14.35.32215\bin\Hostx64\x64\cl.exe" x64
) ELSE (
    call "D:\Programmi\Microsoft Visual Studio\BuildTools2022\VC\Tools\MSVC\14.35.32215\bin\Hostx64\x64\cl.exe" x64
)

if [%1%]==[] (
  nmake /f MakefileWin all
) else (
  nmake /f MakefileWin clean
)