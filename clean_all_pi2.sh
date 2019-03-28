SRC_DIR=$HOME/bmc64-pi2

cd $SRC_DIR/third_party/vice-3.2
make clean
git status | grep deleted: | sed 's/deleted:/git checkout /' | sh

cd $SRC_DIR/third_party/circle-stdlib
find . -name 'config.cache' -exec rm {} \;

git reset --hard
cd libs/circle-newlib
git reset --hard
cd ../circle
git reset --hard

cd $SRC_DIR
make clean
