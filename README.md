# tilepuzzles_android
This is a tiles slider/roller/spinner puzzle game for Android. 
The back-end game implementation is cross platform C++ using @google/filament PBR framework. 
The front-end UI is Android / Kotlin calling C++ / NDK game APIs using JNI. 
The C++ backend can be configured to use OpenGL, Vulkan, Metal (via MoltenVK) across multiple platforms. 
The C++ backend is also compiled to WebAssembly (using Emscripten) to run in a browser.

![androidHexSpinner](https://user-images.githubusercontent.com/45042115/172473723-23def269-ad2c-4ae5-a808-f1c4d18d3d42.gif)
