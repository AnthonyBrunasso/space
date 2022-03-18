@echo off

if not exist build\proto mkdir build\proto
for /f %%f in ('dir /b proto\*.proto') do (
  call third_party\protobuf\build_output\protoc -I=proto\ --cpp_out=build\proto\ %%f
)
