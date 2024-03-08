#!/bin/bash

while getopts ":b:u:d" opt; do
  case $opt in
    b) board="$OPTARG"
    ;;
    u) upload_target="$OPTARG"
    ;;
    d) defines="-D WITH_DEBUG_MESSAGES=1"
    ;;
    \?) echo "Invalid option -$OPTARG" >&2
    exit 1
    ;;
  esac

  case $OPTARG in
    -*) echo "Option $opt needs a valid argument"
    exit 1
    ;;
  esac
done

case $board in
  uno|leonardo|nano|nano328|nano328old)
    echo "board is" $board
    ;;
  *)
    echo "you should specify board via -b option"
    echo "valid boards are: uno nano nano328 nano328old leonardo"
    exit 1
    ;;
esac

if [ -z "$upload_target" ]
then
  rm -rf build/$board
  cmake -S. -Bbuild/$board -D CMAKE_TOOLCHAIN_FILE=cmake/toolchain/$board.toolchain.cmake -D CMAKE_BUILD_TYPE=MinSizeRel $defines
  cmake --build build/$board -j
else
  if [ ! -d "build/$board" ]
  then
    cmake -S. -Bbuild/$board -D CMAKE_TOOLCHAIN_FILE=cmake/toolchain/$board.toolchain.cmake -D CMAKE_BUILD_TYPE=MinSizeRel $defines
  fi
  cmake --build build/$board -j -t upload-$upload_target
fi
