@echo off
call run_constants.bat
set file_arg=%1
for /F "delims=" %%i in ("%file_arg%") do set filename="%%~ni"

cl %1 /O2 /GL /GR- /nologo /Bt /std:c++latest /fp:strict /diagnostics:caret /I . /I src\ /I third_party\ /Fo:%BIN_DIR%\ /DUNICODE /link user32.lib opengl32.lib gdi32.lib third_party\lib\openal32.lib /OUT:%BIN_DIR%/%filename%.exe

if %ERRORLEVEL% EQU 0 (
  if "%~2" == "-r" (
    %BIN_DIR%\%filename%.exe
  )
)
