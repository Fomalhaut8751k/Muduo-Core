#! /bin/bash

set -e

rm -rf  ../build/*

if [ ! -d ../build ]; then
    mkdir ../build
fi

cd  ../build
cmake ..
make
cd ../bin

if [ ! -d /usr/include/mymuduo ]; then
    mkdir /usr/include/mymuduo
fi

for header in `ls ../include/*.h`
do
    cp $header /usr/include/mymuduo
done

cp ../lib/libmymuduo.so /usr/lib

ldconfig