@echo off
call run_constants.bat
set file_arg=%1
for /F "delims=" %%i in ("%file_arg%") do set filename="%%~ni"

cl %1 ^
third_party\imgui\imgui.cpp ^
third_party\imgui\imgui_draw.cpp ^
third_party\imgui\imgui_tables.cpp ^
third_party\imgui\imgui_widgets.cpp ^
third_party\imgui\imgui_demo.cpp ^
third_party\imgui\backends\imgui_impl_opengl3.cpp ^
/Zi /GL /GR- /nologo /EHsc /std:c++17 /fp:strict /diagnostics:caret ^
/I . /I src\ /I third_party\ /I third_party\imgui\ /I third_party\imgui\backends\ ^
/Fo:%BIN_DIR%\ /DUNICODE ^
/link user32.lib opengl32.lib gdi32.lib ^
/OUT:%BIN_DIR%/%filename%.exe

if %ERRORLEVEL% EQU 0 (
  if "%~2" == "-r" (
    %BIN_DIR%\%filename%.exe
  )
)
