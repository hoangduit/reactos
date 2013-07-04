#!/bin/sh

if [ "x$ROS_ARCH" = "x" ]; then
  echo Could not detect RosBE.
  exit 1
fi

BUILD_ENVIRONMENT=MinGW
ARCH=$ROS_ARCH
REACTOS_SOURCE_DIR=$(cd `dirname $0` && pwd)
REACTOS_OUTPUT_PATH=output-$BUILD_ENVIRONMENT-$ARCH

usage() {
echo Invalid parameter given.
exit 1
}

CMAKE_GENERATOR="Ninja"
for (( i=1; i<=$#; i++ )); do
	case ${!i} in
		-D)
			((i++))
			if [[ "x${!i}" == x?*=* ]] ; then
				ROS_CMAKEOPTS+=" -D ${!i}"
			else
				usage
			fi
		;;

		-D?*=*)
			 ROS_CMAKEOPTS+=" ${!i}"
		;;
		makefiles|Makefiles)
			CMAKE_GENERATOR="Unix Makefiles"
		;;
		*)
			usage
	esac
done

if [ "$REACTOS_SOURCE_DIR" = "$PWD" ]; then
  echo Creating directories in $REACTOS_OUTPUT_PATH
  mkdir -p "$REACTOS_OUTPUT_PATH"
  cd "$REACTOS_OUTPUT_PATH"
fi

mkdir -p host-tools reactos

echo Preparing host tools...
cd host-tools
rm -f CMakeCache.txt

REACTOS_BUILD_TOOLS_DIR="$PWD"
cmake -G "$CMAKE_GENERATOR" -DARCH=$ARCH $ROS_CMAKEOPTS "$REACTOS_SOURCE_DIR"

echo Preparing reactos...
cd ../reactos
rm -f CMakeCache.txt

cmake -G "$CMAKE_GENERATOR" -DENABLE_CCACHE=0 -DPCH=0 -DCMAKE_TOOLCHAIN_FILE=toolchain-gcc.cmake -DARCH=$ARCH -DREACTOS_BUILD_TOOLS_DIR="$REACTOS_BUILD_TOOLS_DIR" $ROS_CMAKEOPTS "$REACTOS_SOURCE_DIR"

echo Configure script complete! Enter directories and execute appropriate build commands \(ex: ninja, make, makex, etc...\).
