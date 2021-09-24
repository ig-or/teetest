#!/usr/bin/env bash
cd build1
echo building the sw....
ninja 
if [ $? -eq 0 ] 
then
	echo "NINJA SUCCEEDED"
else
	echo "NINJA FAILED"
	exit 1
fi

#exit 0

echo "loading the software ...."
$TEENSY_TOOLS/teensy_reboot

if [ $? -eq 0 ] 
then
    echo "REBOOT OK"
else
   echo "REBOOT failed"
   cd ..
   exit 1
fi

exit 0