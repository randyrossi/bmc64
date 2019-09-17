#!/bin/sh

SRC_PI2=bmc64-pi2-v39
SRC_PI3=bmc64-pi3-v39
SRC_PI0=bmc64-pi0-v39

echo "Are machines built?"
echo

ls -l ../$SRC_PI2/*.img.* | awk '{print $6" "$7" "$8" "$9}'
ls -l ../$SRC_PI3/*.img.* | awk '{print $6" "$7" "$8" "$9}'
ls -l ../$SRC_PI0/*.img.* | awk '{print $6" "$7" "$8" "$9}'

echo -n "Proceed? (y/N):"
read YN
if [ "$YN" != "Y" -a "$YN" != "y" ]
then
exit
fi

./make_release.sh c64
./make_release.sh vic20
./make_release.sh c128
./make_release.sh plus4
