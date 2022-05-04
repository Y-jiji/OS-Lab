echo "=== RAM ==="
mknod /dev/myram b 1 13
ls /dev/ | grep ram
buildmyram 512 /dev/myram
mkfs.mfs /dev/myram
mount /dev/myram /root/myramtest
df
read -n 1 -p "Press any key to continue..."

echo "=== Disk ==="
rm -r -f /root/mydisktest
mkdir /root/mydisktest
read -n 1 -p "Press any key to continue..."

echo "=== Start Test ==="
clang -o test test.c # compile
./test               # run

echo "==========================="
echo "========== done ==========="
echo "==========================="
