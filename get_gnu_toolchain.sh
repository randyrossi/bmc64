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

# Node.js runs the web UI's JavaScript tests (tools/webui_test), which the
# build requires to pass. A Node.js on PATH that is new enough is used as it
# is; otherwise the pinned release is downloaded and its checksum verified.
NODE_VERSION=24.21.0
NODE_MIN_VERSION=22.12.0   # loads ES modules from .js files without any flags
NODE_NAME=node-v${NODE_VERSION}-linux-x64
NODE_ARCHIVE=${NODE_NAME}.tar.xz
NODE_URL=https://nodejs.org/dist/v${NODE_VERSION}/${NODE_ARCHIVE}
NODE_SHA256=fd8e59d5a511510f6a298afb548f18c7d2b1be404d8b4a27d94fbe49f56cb2d6
NODE_HOME=${BMC64_NODE_HOME:-$TOOLCHAIN_CACHE/$NODE_NAME}

# $1 is a node binary; true if it runs and is at least NODE_MIN_VERSION.
node_is_usable()
{
    [ -x "$1" ] || return 1
    local have
    have=$("$1" --version 2>/dev/null) || return 1
    have=${have#v}
    [ "$(printf '%s\n%s\n' "$NODE_MIN_VERSION" "$have" | sort -V | head -n 1)" = "$NODE_MIN_VERSION" ]
}

node_install()
{
    local archive_path=$TOOLCHAIN_CACHE/$NODE_ARCHIVE
    local install_path=$TOOLCHAIN_CACHE/.${NODE_NAME}.installing

    mkdir -p "$TOOLCHAIN_CACHE" || return 1
    if ! { [ -f "$archive_path" ] &&
           [ "$(sha256sum "$archive_path" 2>/dev/null | cut -d' ' -f1)" = "$NODE_SHA256" ]; }
    then
        rm -f "$archive_path"
        echo "Downloading Node.js v$NODE_VERSION..."
        if command -v curl >/dev/null 2>&1
        then
            curl --fail --location --retry 3 --output "$archive_path" "$NODE_URL" || return 1
        elif command -v wget >/dev/null 2>&1
        then
            wget --output-document="$archive_path" "$NODE_URL" || return 1
        else
            echo "curl or wget is required to download Node.js." >&2
            return 1
        fi
        if [ "$(sha256sum "$archive_path" 2>/dev/null | cut -d' ' -f1)" != "$NODE_SHA256" ]
        then
            echo "Node.js download does not match its expected checksum." >&2
            rm -f "$archive_path"
            return 1
        fi
    fi

    echo "Installing Node.js v$NODE_VERSION..."
    rm -rf "$install_path"
    mkdir -p "$install_path" || return 1
    tar -xJf "$archive_path" -C "$install_path" || return 1
    rm -rf "$NODE_HOME"
    mv "$install_path/$NODE_NAME" "$NODE_HOME" || return 1
    rmdir "$install_path"
}

setup_node()
{
    local path_node
    path_node=$(command -v node 2>/dev/null || true)
    if node_is_usable "$path_node"
    then
        echo "Using Node.js $("$path_node" --version): $path_node"
        return 0
    fi

    if ! node_is_usable "$NODE_HOME/bin/node"
    then
        node_install || true
    fi
    if ! node_is_usable "$NODE_HOME/bin/node"
    then
        echo "Node.js $NODE_MIN_VERSION or newer is required to run the web UI tests, and could not be installed." >&2
        return 1
    fi

    export NODE_HOME
    case ":$PATH:" in
        *":$NODE_HOME/bin:"*) ;;
        *) export PATH=$NODE_HOME/bin:$PATH ;;
    esac
    echo "Using Node.js v$NODE_VERSION: $NODE_HOME"
}

if ! setup_node
then
    return 1 2>/dev/null || exit 1
fi
