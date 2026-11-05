@echo off
IF NOT EXIST ..\build mkdir ..\build
pushd build
cl -FC -Zi ..\code\win32_game_loop.cpp /link user32.lib gdi32.lib winmm.lib -incremental:no
popd
