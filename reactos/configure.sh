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
while [ $# -gt 0 ]; do
	case $1 in
		-D)
			shift
			if echo "x$1" | grep 'x?*=*' > /dev/null; then
				ROS_CMAKEOPTS=$ROS_CMAKEOPTS" -D $1"
			else
				usage
			fi
		;;

		-D?*=*|-D?*)
			ROS_CMAKEOPTS=$ROS_CMAKEOPTS" $1"
		;;
		makefiles|Makefiles)
			CMAKE_GENERATOR="Unix Makefiles"
		;;
		*)
			usage
	esac

	shift
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
cmake -G "$CMAKE_GENERATOR" -DARCH:STRING=$ARCH $ROS_CMAKEOPTS "$REACTOS_SOURCE_DIR"

echo Preparing reactos...
cd ../reactos
rm -f CMakeCache.txt

cmake -G "$CMAKE_GENERATOR" -DENABLE_CCACHE:BOOL=0 -DCMAKE_TOOLCHAIN_FILE:FILEPATH=toolchain-gcc.cmake -DARCH:STRING=$ARCH -DREACTOS_BUILD_TOOLS_DIR:PATH="$REACTOS_BUILD_TOOLS_DIR" $ROS_CMAKEOPTS "$REACTOS_SOURCE_DIR"

echo Configure script complete! Enter directories and execute appropriate build commands \(ex: ninja, make, makex, etc...\).
