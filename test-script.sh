echo "=== RAM ==="
mknod /dev/myram b 1 13
ls /dev/ | grep ram
buildmyram 510 /dev/myram
mkfs.mfs /dev/myram
rm -r -f /root/myramtest
mkdir /root/myramtest
mount /dev/myram /root/myramtest
df

echo "=== Disk ==="
rm -r -f /root/mydisktest
mkdir /root/mydisktest

echo "=== Start Test ==="
clang -o test test.c # compile
./test               # run

echo "==========================="
echo "========== done ==========="
echo "==========================="




