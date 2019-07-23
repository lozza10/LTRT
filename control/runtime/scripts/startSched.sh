#!/bin/bash
#
# startSched.sh
# LTRT schedule control
#
# Version 1.03
# LT 06/13
#

sched_file="../schedules/$1"
sched_line=$2
ac_script="analogueCapture"
ac_status=`ps -C $ac_script > /dev/null; echo $?`

# Check arguments
if [ $# != 2 ]; then
  echo
  echo -e "   startSched.sh :: Requires schedule file and line number as arguments\n"
  exit
fi

if [ ! -f $sched_file ]; then
  echo
  echo -e "   startSched.sh :: $sched_file does not exist...exiting\n"
  exit
fi

if [ $ac_status -eq 0 ]; then
  echo
  echo -e "   startSched.sh :: $ac_script is already running...exiting\n"
  exit
fi

# Run
execline=`cat $sched_file | grep "#$sched_line" | awk -F"#$sched_line" '{print$2}'`

if [ "$execline" == "" ]; then
  echo
  echo -e "   startSched.sh :: Line $sched_line does not exist...exiting\n"
  exit
fi

$ac_script $execline

echo -e "   startSched.sh :: No more lines...schedule complete\n"

