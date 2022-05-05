echo "=== Compile [test.c] ==="
clang -o test test.c # compile
echo "finished"

echo "=== RAM ==="
mknod /dev/myram b 1 13
ls /dev/ | grep ram
buildmyram 756 /dev/myram
mkfs.mfs /dev/myram
rm -r -f /usr/myramtest
mkdir /usr/myramtest
mount /dev/myram /usr/myramtest
df
echo "finished"

echo "=== Disk ==="
rm -r -f /usr/mydisktest
mkdir /usr/mydisktest
echo "finished"

echo "=== Start Test ==="
./test               # run

echo "==========================="
echo "========== done ==========="
echo "==========================="
