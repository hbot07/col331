#!/bin/bash

submission=$1
# if test_dir already exixts, delete it
if [ -d "./test_dir" ]; then
	rm -rf ./test_dir
fi
mkdir ./test_dir

echo $submission

# Accept both .tar.gz and .gz or any other gz ;)
fileNameRegex="lab2_[0-9]{4}[A-Z]{2}.[0-9]{4}.*.gz"

if ! [[ $submission =~ $fileNameRegex ]]; then
	# not considering zip for now
	echo "File doesn't match the naming convention"
	exit
fi

# Extract the Entry number
entryNoRegex="[0-9]{4}[A-Z]{2}.[0-9]{4}"
if [[ $submission =~ $entryNoRegex ]]; then
	entryNum=${BASH_REMATCH[0]}
	echo "Entry Number: $entryNum"
fi

echo "Setting the test directory"

tar -xzvf "$submission" -C ./test_dir
cp out* *.sh ./test_dir
cd ./test_dir

# Change the gcc binary
sed -i 's/gcc-10\|gcc-9/gcc/g' Makefile

echo "Executing the test cases"

pkill qemu-system-x86
pkill qemu-system-i386
make clean

make xv6.img
#make

arr=(1 2 3 4 5)
# arr=(0)
log_header="logflag.h"
RECOV=0

for idx in "${!arr[@]}"; do
    echo "Running...${arr[idx]}"

    echo 00000 > hello.txt
    make fs.img > /dev/null

    sed -i "s/\(#define LOG_FLAG \)\(-1\|[0-9]\+\)/\1${arr[idx]}/" "$log_header"
    # cat ${log_header}
    timeout 30s ./test_lab2.sh 1 | grep "LOG" > res_panic_$idx

    sed -i "s/\(#define LOG_FLAG \)\(-1\|[0-9]\+\)/\1${RECOV}/" "$log_header"
    # cat ${log_header}
    timeout 30s ./test_lab2.sh 1 | grep "LOG" > res_recovery_$idx
done

output_dir="../outputs"
check_test=${#arr[@]}
total_test=0

echo "" > .output

marks="$entryNum"
for ((t=0;t<$check_test;++t))
do
	echo -n "Test #${t}: "

    diff -iZwB ${output_dir}/out_panic_$t res_panic_$t > /dev/null
    res1=$?

    diff -iZwB ${output_dir}/out_recovery_$t res_recovery_$t > /dev/null
    res2=$?

    if [ "$res1" -eq 0 ] && [ "$res2" -eq 0 ];
	then
		echo -e "\e[0;32mPASS\e[0m"
		marks="$marks,1" # 1 mark for pass
		((total_test++))
	else
		echo -e "\e[0;31mFAIL\e[0m"
		marks="$marks,0" # 0 mark for fail
		echo "Output for test case $t:" >> .output
        echo "---------panic----------" >> .output
		cat res_panic_$t >> .output
        echo "--------recovery--------" >> .output
		cat res_recovery_$t >> .output
	fi
done
echo "$total_test test cases passed for $entryNum"
echo $marks >> ../marks.csv
