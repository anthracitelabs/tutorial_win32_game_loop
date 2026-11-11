@echo off
IF NOT EXIST build mkdir build
pushd build
copy %SDL_HOME%\lib\x64\SDL3.dll .

:: COMMON LINKER SWITCHES
set link=                   -opt:ref               &:: Remove unused functions
set link=%link%             -incremental:no        &:: Perform full link each time

:: No optimizations (slow): -Od; all optimizations (fast): -O2
cl -FC -Zi -Od /I "%SDL_HOME%/include" ..\code\sdl3_game_loop.cpp "%SDL_HOME%/lib/x64/sdl3.lib" opengl32.lib /link %link%

popd
