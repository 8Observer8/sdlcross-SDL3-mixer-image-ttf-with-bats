cmake -G "MinGW Makefiles" -S . -B dist/win ^
-DSDL3_DIR=libs/SDL-3.3.0-mingw/lib/cmake/SDL3 ^
-DSDL3_image_DIR=libs/SDL3_image-devel-3.2.4-mingw/lib/cmake/SDL3_image ^
-DSDL3_mixer_DIR=libs/SDL3_mixer-3.1.0-mingw/lib/cmake/SDL3_mixer ^
-DSDL3_ttf_DIR=libs/SDL3_ttf-devel-3.2.2-mingw/lib/cmake/SDL3_ttf ^
-DCMAKE_BUILD_TYPE=Debug ^
-DWITH_IMAGE=ON ^
-DWITH_MIXER=ON ^
-DWITH_TTF=ON
