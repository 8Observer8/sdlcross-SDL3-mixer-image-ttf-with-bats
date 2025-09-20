cd dist\web
cmake --build . -j4

cd ..\..
mkdir public\js
set current_dir=%~dp0
copy "%current_dir%dist\web\sdlcross.wasm" "%current_dir%public\js"
copy "%current_dir%dist\web\sdlcross.js" "%current_dir%public\js"
