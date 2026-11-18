## Implementing Game Loop

This is a demonstration of a basic game loop implementation in win32 & SDL. VSYNC should be enabled (possibly with OpenGL) to match display refresh rate. Sleep is not recommended as it is unreliable.

Source is the famous Glenn Fiedler article on Gaffer on Games website.

[Gaffer on Games - Fix your timestep!](https://gafferongames.com/post/fix_your_timestep/)

In the early days of the project, handmade.cpp was directly included in win32_handmade.cpp via #include "handmade.cpp"

However, as the architecture evolved to support live code editing (hot-reloading), this structure changed


Phase 1: The Unity Build (Early Days)

Initially, Casey compiled the entire project as a single translation unit (a "Unity Build"). To achieve this, the platform layer textually pasted the game code right into itself:
• win32_handmade.cpp explicitly used #include "handmade.cpp".
• The compiler handled it as one giant file, allowing quick compilation without a complex build system.

Phase 2: DLL Separation for Hot-Reloading (Later Days)

When live loop editing and hot-reloading were introduced, win32_handmade.cpp could no longer directly include handmade.cpp. They had to be split into two separate compilation units so the game could be compiled into a separate dynamic library (handmade.dll):
• win32_handmade.cpp: Compiles into win32_handmade.exe. It dropped the direct inclusion of handmade.cpp. Instead, it includes handmade_platform.h to understand the shared structs and function pointers needed to communicate with the game.
• handmade.cpp: Compiles independently into handmade.dll. The Win32 platform layer unloads and reloads this DLL at runtime whenever it detects a fresh build
