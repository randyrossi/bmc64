#!/bin/bash

if [ "$1" = "" ]
then
echo "Need version num."
exit
fi

if [ "$2" = "" ]
then
echo "Need machine or 'all'"
exit
fi

cd /home/rrossi
if [ "$2" = "c64" -o "$2" = "all" ]
then
echo "Install c64"
mv bmc64*files.zip /home/webadmin/accentual.com/bmc64/downloads
mv bmc64*img.zip /home/webadmin/accentual.com/bmc64/downloads
MD564F=`md5sum /home/webadmin/accentual.com/bmc64/downloads/bmc*${1}*.files.zip | awk '{print $1}'`
MD564I=`md5sum /home/webadmin/accentual.com/bmc64/downloads/bmc*${1}*.img.zip | awk '{print $1}'`
cd /home/webadmin/accentual.com/bmc64
cp index.html index.html.bak
cp index.html.template tmp.html
perl -pi -e "s@%VERSION%@$1@g" tmp.html
perl -pi -e "s@%MD564F%@$MD564F@g" tmp.html
perl -pi -e "s@%MD564I%@$MD564I@g" tmp.html
mv tmp.html index.html
fi

cd /home/rrossi
if [ "$2" = "c128" -o "$2" = "all" ]
then
echo "Install c128"
mv bmc128*files.zip /home/webadmin/accentual.com/bmc128/downloads
mv bmc128*img.zip /home/webadmin/accentual.com/bmc128/downloads
MD5128F=`md5sum /home/webadmin/accentual.com/bmc128/downloads/bmc*${1}*.files.zip | awk '{print $1}'`
MD5128I=`md5sum /home/webadmin/accentual.com/bmc128/downloads/bmc*${1}*.img.zip | awk '{print $1}'`
cd /home/webadmin/accentual.com/bmc128
cp index.html index.html.bak
cp index.html.template tmp.html
perl -pi -e "s@%VERSION%@$1@g" tmp.html
perl -pi -e "s@%MD5128F%@$MD5128F@g" tmp.html
perl -pi -e "s@%MD5128I%@$MD5128I@g" tmp.html
mv tmp.html index.html
fi

cd /home/rrossi
if [ "$2" = "vic20" -o "$2" = "all" ]
then
echo "Install vic20"
mv bmvic20*files.zip /home/webadmin/accentual.com/bmvic20/downloads
mv bmvic20*img.zip /home/webadmin/accentual.com/bmvic20/downloads
MD5V20F=`md5sum /home/webadmin/accentual.com/bmvic20/downloads/bmv*${1}*.files.zip | awk '{print $1}'`
MD5V20I=`md5sum /home/webadmin/accentual.com/bmvic20/downloads/bmv*${1}*.img.zip | awk '{print $1}'`
cd /home/webadmin/accentual.com/bmvic20
cp index.html index.html.bak
cp index.html.template tmp.html
perl -pi -e "s@%VERSION%@$1@g" tmp.html
perl -pi -e "s@%MD5V20F%@$MD5V20F@g" tmp.html
perl -pi -e "s@%MD5V20I%@$MD5V20I@g" tmp.html
mv tmp.html index.html
fi

cd /home/rrossi
if [ "$2" = "plus4" -o "$2" = "all" ]
then
echo "Install plus4"
mv bmplus4*files.zip /home/webadmin/accentual.com/bmplus4/downloads
mv bmplus4*img.zip /home/webadmin/accentual.com/bmplus4/downloads
MD5PLUS4F=`md5sum /home/webadmin/accentual.com/bmplus4/downloads/bmp*${1}*.files.zip | awk '{print $1}'`
MD5PLUS4I=`md5sum /home/webadmin/accentual.com/bmplus4/downloads/bmp*${1}*.img.zip | awk '{print $1}'`
cd /home/webadmin/accentual.com/bmplus4
cp index.html index.html.bak
cp index.html.template tmp.html
perl -pi -e "s@%VERSION%@$1@g" tmp.html
perl -pi -e "s@%MD5PLUS4F%@$MD5PLUS4F@g" tmp.html
perl -pi -e "s@%MD5PLUS4I%@$MD5PLUS4I@g" tmp.html
mv tmp.html index.html
fi
