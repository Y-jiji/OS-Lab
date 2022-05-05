echo "=== Compile [test.c] ==="
clang -o test test.c # compile
echo "finished"

echo "=== RAM ==="
mknod /dev/myram b 1 13
ls /dev/ | grep ram
buildmyram 450 /dev/myram
mkfs.mfs /dev/myram
rm -r -f /root/myramtest
mkdir /root/myramtest
mount /dev/myram /root/myramtest
df
echo "finished"

echo "=== Disk ==="
rm -r -f /root/mydisktest
mkdir /root/mydisktest
echo "finished"

echo "=== Start Test ==="
./test               # run

echo "==========================="
echo "========== done ==========="
echo "==========================="
