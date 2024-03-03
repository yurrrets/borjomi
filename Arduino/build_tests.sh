#!/bin/bash

rm -rf build/tests
cmake -D CMAKE_BUILD_TYPE=Debug -S./tests -Bbuild/tests 
cmake --build build/tests -j

