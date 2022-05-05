echo "=== Compile [test.c] ==="
clang -o test test.c # compile
echo "finished"

echo "=== RAM ==="
mknod /dev/myram b 1 13
ls /dev/ | grep ram
buildmyram 450 /dev/myram
mkfs.mfs /dev/myram
rm -r -f /myramtest
mkdir /myramtest
mount /dev/myram /myramtest
df
echo "finished"

echo "=== Disk ==="
rm -r -f /mydisktest
mkdir /mydisktest
echo "finished"

echo "=== Start Test ==="
./test               # run

echo "==========================="
echo "========== done ==========="
echo "==========================="
