@echo off

if not defined VisualStudioVersion (
    if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat" (
        call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
    ) else (
        call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
    )
)

IF NOT EXIST "build\" mkdir build
cd build
IF NOT EXIST build.ninja cmake ..\ -G Ninja
ninja && src\space.exe
cd ..\
