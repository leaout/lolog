#!/bin/bash

if [ $# != 1 ] ; then
  echo "USAGE: $0 release|debug"
  echo " e.g.: $0 release"
  exit 1;
fi

CPU_CORES=$(grep ^cpu\\scores /proc/cpuinfo | uniq |  awk '{print $4}')

#build protos
SCRIPT_PATH="$(
  cd "$(dirname "$0")"
  pwd -P
)"

# build
#create dir if not exist
release_dir="${SCRIPT_PATH}/cmake-build-release"
debug_dir="${SCRIPT_PATH}/cmake-build-debug"
if [ ! -d ${release_dir} ];then
  mkdir -p ${release_dir}
fi

if [ ! -d ${debug_dir} ];then
  mkdir -p ${debug_dir}
fi

if [ $1 = "release" ]; then
  cd ${release_dir}
  cmake -DCMAKE_BUILD_TYPE=Release  -DBUILD_TEST=ON .. && cmake --build . -- -j $CPU_CORES
else
  cd ${debug_dir}
  cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TEST=ON .. && cmake --build . -- -j $CPU_CORES
fi
