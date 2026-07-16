@echo off
IF NOT EXIST ..\build mkdir ..\build
pushd ..\build
cl ..\code\win32_game_loop.cpp /link user32.lib gdi32.lib winmm.lib -incremental:no
popd
