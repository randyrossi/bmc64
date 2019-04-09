#!/bin/sh
# Convert standard input to a C char array, write to a file, then create an
# MD5 sum of that file and append said MD5 sum as char array to the file.

FILE=$1

# Filter out whitespace, empty lines, and comments.
sanitize() {
    sed 's/^[[:space:]]*//; /^$/d; /^\/\//d'
}

# Convert stdin to a \0-terminated char array.
dump() {
    printf 'static const char %s[] = {\n' $1
    od -v -A n -t x1 | sed 's/[[:space:]]*\([[:alnum:]]\{2\}\)/0x\1, /g'
    printf '0x00 };\n'
}

# Print MD5 hash w/o newline character to not embed the character in the array.
hash() {
    # md5sum is not standard, so try different platform-specific alternatives.
    { md5sum $1 2> /dev/null || md5 -q $1 || digest -a md5 $1; } |
        cut -b -32 | tr -d '\n\r'
}

trap "rm -f $FILE.temp" EXIT

sanitize | tee $FILE.temp |
    dump x264_opencl_source > $FILE

hash $FILE.temp |
    dump x264_opencl_source_hash >> $FILE
