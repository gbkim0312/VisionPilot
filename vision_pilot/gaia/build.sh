#!/bin/bash
# build.sh -- builds vp SecurityPlatform files
#   by Jinwoo Lee <jwlee@vp.io>

# This script should run inside securityplatform root directory.
# Default options:
# - BUILD_TYPE:debug
#
# Usage:
# - ./build.sh
#  - build with default options
# - ./build.sh --help
#  - print help

set -o errexit
set -o nounset

CMAKE=cmake
BUILD=./build
GENERATOR="Unix Makefiles"
TYPE=Debug
BUILD_DIR=$BUILD/$TYPE
CLEAN=
RESET=
VERBOSE=
GAIA_SHARED="ON"
STANDARD_TYPE=""

Help()
{
   # Display Help
   echo "Usage: ./$(basename $0)"
   echo "  Default options: "
   echo "    BUILD_TYPE:debug"
   echo
   echo "  options:"
   echo "    -h, --help        Print this help."
   echo "    -v, --verbose     Print build information as much as possible"
   echo "    -s, --static      Build static libraries (default: shared)."
   echo "    --standard=STANDARD"
   echo "                      set STANDARD as a target standard."
   echo "                      candidates: [ieee1609.2.1, ydt3957, etsi102941]"
   echo
   echo "  command:"
   echo "    [build types]"
   echo "      debug             Build with Debug option."
   echo "      release           Build with Release option."
   echo "      relwdeb           Build with RelWithDebInfo option."
   echo "    [pre-build options]"
   echo "      clean             To do a clean build."
   echo "      reset             To reset the build directory and then build."
}

for arg; do
  case "${arg%%=*}" in
    --help|-h)    Help; exit 0;;
    -v|--verbose) VERBOSE='VERBOSE=1' ;;
    -s|--static)  GAIA_SHARED='OFF' ;;
    --standard)   STANDARD_TYPE="${arg##*=}" ;;
    debug)        TYPE=Debug ;;
    release)      TYPE=Release ;;
    relwdeb)      TYPE=RelWithDebInfo ;;
    clean)        CLEAN=1 ;;
    reset)        RESET=1 ;;
   *)             echo "unknown option $arg\n" >&2; exit 1 ;;
  esac
done

STANDARD_TYPE_LIST=(
  "ieee1609.2.1"
  "ydt3957"
  "etsi102941"
)

# Validate standard type if provided
if [[ -n "$STANDARD_TYPE" ]]; then
  if [[ ! " ${STANDARD_TYPE_LIST[@]} " =~ " ${STANDARD_TYPE} " ]]; then
    echo "Invalid standard type: ${STANDARD_TYPE}"
    echo "Valid options are: ${STANDARD_TYPE_LIST[*]}"
    exit 1
  fi
fi

BUILD_DIR=$BUILD/$STANDARD_TYPE/$TYPE

if ! command -v ninja &> /dev/null
then
    echo "Ninja could not be found. Can be installed by using 'apt install ninja-build'"
    echo "Using make"
else
    GENERATOR=Ninja
fi

[[ -n $RESET && -d $BUILD_DIR ]] && rm -rf $BUILD_DIR
$CMAKE -G "$GENERATOR" -S . -B $BUILD_DIR -Wno-dev \
  -DCMAKE_INSTALL_PREFIX=$BUILD_DIR \
  -DCMAKE_BUILD_TYPE=$TYPE \
  -DGAIA_SHARED_LIBS=$GAIA_SHARED \
  -DSTANDARD_TYPE=${STANDARD_TYPE}

[[ -n $CLEAN ]] && $CMAKE --build $BUILD_DIR --target clean
$CMAKE --build $BUILD_DIR -- $VERBOSE install -j16
