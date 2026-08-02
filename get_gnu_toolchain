#!/bin/bash

# Source this file to configure the Arm GNU Toolchain required by BMC64.
TOOLCHAIN_VERSION=15.2.1
TOOLCHAIN_RELEASE=15.2.rel1
TOOLCHAIN_NAME=arm-gnu-toolchain-${TOOLCHAIN_RELEASE}-x86_64-arm-none-eabi
TOOLCHAIN_ARCHIVE=${TOOLCHAIN_NAME}.tar.xz
TOOLCHAIN_URL=https://gitlab.arm.com/api/v4/projects/tooling%2Fgnu-toolchains-for-arm/packages/generic/gnu-toolchain/${TOOLCHAIN_RELEASE}/${TOOLCHAIN_ARCHIVE}
TOOLCHAIN_CACHE=${XDG_CACHE_HOME:-$HOME/.cache}/bmc64/toolchains
TOOLCHAIN_HOME=${BMC64_TOOLCHAIN_HOME:-$TOOLCHAIN_CACHE/$TOOLCHAIN_NAME}

toolchain_is_valid()
{
    [ -x "$1/bin/arm-none-eabi-gcc" ] &&
        [ "$("$1/bin/arm-none-eabi-gcc" -dumpfullversion 2>/dev/null)" = "$TOOLCHAIN_VERSION" ]
}

if toolchain_is_valid "${ARM_HOME:-}"
then
    TOOLCHAIN_HOME=$ARM_HOME
elif command -v arm-none-eabi-gcc >/dev/null 2>&1
then
    path_compiler=$(readlink -f "$(command -v arm-none-eabi-gcc)")
    path_toolchain=$(dirname "$(dirname "$path_compiler")")
    if toolchain_is_valid "$path_toolchain"
    then
        TOOLCHAIN_HOME=$path_toolchain
    fi
fi

if ! toolchain_is_valid "$TOOLCHAIN_HOME"
then
    mkdir -p "$TOOLCHAIN_CACHE"
    archive_path=$TOOLCHAIN_CACHE/$TOOLCHAIN_ARCHIVE

    if ! tar -tJf "$archive_path" >/dev/null 2>&1
    then
        rm -f "$archive_path"
        echo "Downloading Arm GNU Toolchain $TOOLCHAIN_RELEASE..."
        if command -v curl >/dev/null 2>&1
        then
            curl --fail --location --retry 3 --output "$archive_path" "$TOOLCHAIN_URL"
        elif command -v wget >/dev/null 2>&1
        then
            wget --output-document="$archive_path" "$TOOLCHAIN_URL"
        else
            echo "get_gnu_toolchain requires curl or wget to download the toolchain." >&2
            return 1 2>/dev/null || exit 1
        fi
    fi

    install_path=$TOOLCHAIN_CACHE/.${TOOLCHAIN_NAME}.installing
    rm -rf "$install_path"
    mkdir -p "$install_path"
    echo "Installing Arm GNU Toolchain $TOOLCHAIN_RELEASE..."
    tar -xJf "$archive_path" -C "$install_path"
    rm -rf "$TOOLCHAIN_HOME"
    mv "$install_path/$TOOLCHAIN_NAME" "$TOOLCHAIN_HOME"
    rmdir "$install_path"
fi

if ! toolchain_is_valid "$TOOLCHAIN_HOME"
then
    echo "Arm GNU Toolchain $TOOLCHAIN_VERSION was not installed correctly: $TOOLCHAIN_HOME" >&2
    return 1 2>/dev/null || exit 1
fi

export ARM_HOME=$TOOLCHAIN_HOME
export ARM_VERSION=$TOOLCHAIN_VERSION
case ":$PATH:" in
    *":$ARM_HOME/bin:"*) ;;
    *) export PATH=$ARM_HOME/bin:$PATH ;;
esac

echo "Using Arm GNU Toolchain $ARM_VERSION: $ARM_HOME"