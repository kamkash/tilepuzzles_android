# tilepuzzles_android
This is a tiles slider/roller/spinner puzzle game for Android. 
The back-end game implementation is cross platform C++ using google/filament PBR framework. 
The front-end UI is Android / Kotlin calling C++ / NDK game APIs using JNI. 
The C++ backend can be configured to use OpenGL, Vulkan, Metal (via MoltenVK) across multiple platforms. 
The C++ backend is also compiled to WebAssembly (using Emscripten) to run in a browser.


![androidHexSpinner](https://user-images.githubusercontent.com/45042115/172475313-7ea269b5-9918-46f3-b403-6bd3dee927c9.gif)
![androidSlider](https://user-images.githubusercontent.com/45042115/172475342-a6c090a4-9265-44a9-969c-7936f33ce430.gif)
![androidRoller](https://user-images.githubusercontent.com/45042115/172475802-01aa4a0f-57dc-484b-aea2-0afb594e0f7d.gif)
