# list recipes
default:
    @just --list

img:
    docker build -t trigger -< Dockerfile

build:
    docker run --rm -v $PWD:/app -w /app -u $(id -u):$(id -g) trigger just pkg

enter:
    docker run --rm -it -v $PWD:/app -w /app -u $(id -u):$(id -g) trigger /bin/bash

cmake:
    #!/bin/bash
    export DESTDIR=out/libtrigger
    cmake -B build -S src
    cmake --build build
    cmake --install build

pkg: cmake
    cp -pr DEBIAN out/libtrigger
    dpkg-deb --build out/libtrigger

clean:
    rm -rf build out

clean-img:
    docker rmi trigger