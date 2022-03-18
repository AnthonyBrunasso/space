@echo off

if not defined VisualStudioVersion (
    if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat" (
        call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
    ) else (
        call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
    )
)
if not exist "third_party\protobuf\build_output" call tools\win\build_protobuf.bat
if not exist "build\" mkdir build
if not exist "build\proto" call tools\win\gen_proto.bat
cd build
if not exist build.ninja cmake ..\ -G Ninja
ninja && src\space.exe
cd ..\
