# tilepuzzles_android
This is a tiles slider/roller/spinner puzzle game for Android. 
The back-end game implementation is cross platform C++ using @google/filament PBR framework. 
The front-end UI is Android / Kotlin calling C++ / NDK game APIs using JNI. 
The C++ backend can be configured to use OpenGL, Vulkan, Metal across multiple platforms. 
The C++ backend is also compiled to WebAssembly (using Emscripten) to run in a browser.
