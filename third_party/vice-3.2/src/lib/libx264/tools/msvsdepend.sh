#!/bin/sh
# There's a lot of things going on here
# expected arguments are $(CC) $(CFLAGS) $(SRC) $(OBJ)
# 1) start the dependency line with the object argument
# 2) need to add -Zs -showIncludes to the flags to have the compiler output list of include files without compilation
# 3) look for notes in the output that start with "Note: including file:"
# 4) retain only the filepath from the notes
# 5) convert \ foldername separators to /
# 6) escape spaces in the filepath
# 7) remove system includes (hack: check for "/Program Files" string in filepath)
# 8) sort and remove duplicate filepath entries
# 9) convert newlines to spaces to collapse the dependencies into the one dependency line
# 10) print a newline character, to properly separate dependency lines
echo -n "$4: "
$1 $2 $3 -Zs -showIncludes 2>&1 |
    grep '^Note: including file:' |
    sed 's/^Note: including file:[[:space:]]*\(.*\)$/\1/; s/\\/\//g; s/ /\\ /g' |
    sed '/\/[Pp]rogram\\ [Ff]iles/d' |
    sort | uniq |
    tr -s '\n\r' ' '
echo ''
