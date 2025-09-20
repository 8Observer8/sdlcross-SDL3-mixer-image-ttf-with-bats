All libs are included to this repo.

##Demos:

- Demo in the browser: https://sdlcross-sdl3-mixer-image-ttf-bats.netlify.app/ (zip - 1.78 MB, unzipped - 3.71 MB)
- [Download EXE](https://www.dropbox.com/scl/fi/9tz7xza6ztpv21mdaca1w/sdlcross-SDL3-mixer-image-ttf-with-bats-exe.zip?rlkey=2ab5dcbm83xaa14eoxu1gzeuu&st=r1eadwzx&raw=1) ()
- [Download APK](https://www.dropbox.com/scl/fi/h6iqy50814a89redd15i3/app-debug.apk?rlkey=7l2eyypanzvenylssrdi85w1e&st=8rhydqb0&raw=1) - (apk - 6.4 MB, installed - 12.45 MB)

##Android:

- Download and unzip the [Android](https://www.mediafire.com/file/zt5n2q5hu70u94g/Android.zip/file) folder (zip - 2.88 GB, unzipped - 7.99 GB)
- Create ANDROID_HOME variable with (for example) "H:\Android\SDK" value
- Create ANDROID_NDK_HOME variable with (for example) "H:\Android\SDK\ndk" value

###Building for Android using batch files:

- install-apk
- build-apk

##Windows:

- Download [MinGW GCC 11.2](https://www.mediafire.com/file/wqf5m5o2wyamjaa/mingw1120_64-571mb.zip/file) folder and unzip somewhere on your hard disk
- Install CMake: https://cmake.org/download/

###Building for Windows using batch files:

- config-exe
- build-exe
- run-exe

##WebAssembly:

- Install emsdk: https://emscripten.org/docs/getting_started/downloads.html

###Building for WebAssembly using batch files:

- config-web
- build-web

The "public" folder will be created in the root project folder that can be hosted on Netlify, GitHub Pages, BitBucket Pages and so on.
