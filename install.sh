

#!/bin/sh

cd `dirname $0`

rm -rf .build_myspace

mkdir -p .build_myspace

cd .build_myspace

cmake ..

cmake --build . --target install

cd ..

rm -rf .build_myspace

