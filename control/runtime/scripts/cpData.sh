#!/bin/bash
#
# cpData.sh
# Copies data from telescope control to external backup,
# then deletes the source directory if modified > 5 mins ago.
# This script is ran from cron every 30 minutes.
#
# Version 1.03
# LT 06/13
#

src_dir=/home/pulsar/LTRT/control/data
tgt_dir=/home/pulsar/Data/pulsar_master/LTRT

data=`find $src_dir -type d -name "pos*" -amin +5`

if [ ! $data ]; then
  echo
  echo -e " cpData.sh :: Nothing to copy...exiting\n"
  exit
fi

for line in $data
  do

    # get pos index
    suff=`echo $line | awk -F"pos" '{print$2}'`
    pos=`echo $suff | awk -F"_" '{print$1}'`

    # copy to backup data directory
    rsync -avzt --exclude '*.tmp' $line "$tgt_dir/pos$pos/"

    # delete if copied ok
    stat=`echo $?`
    if [ $stat -eq 0 ]; then 
     rm -r $line
     echo " cpData.sh :: $line copied and removed from source"
    fi

  done
