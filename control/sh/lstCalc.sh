#!/bin/bash
#
# lstCalc.sh
# Convert fractional hrs to LST in degrees and hhmmss formats
#
# LT 06/03/13
#

lst=$1
fhrs=$2

function usage() {
  echo
  echo -e " > lstCalc.sh <\n"
  echo -e " Convert fractional hrs to LST in degrees and hhmmss formats\n"
  echo -e " Usage: lstCalc.sh <LST start (hh:mm:ss)> <fractional hrs (eg. 1.3)>\n"
  echo
}

function format() {
  hhlst=$(echo "$lst" | awk -F: '{print$1}')
  mmlst=$(echo "$lst" | awk -F: '{print$2}')
  sslst=$(echo "$lst" | awk -F: '{print$3}')
  hrs=$(echo "$fhrs" | awk -F. '{print$1}')
  fhh=$(echo "$fhrs" | awk -F. '{print$2}')
  mm=$(echo "0.$fhh*60" | bc)
  fmm=$(echo "$mm" | awk -F. '{print$2}')
  ss=$(echo "0.$fmm*60" | bc)
  lsthh=$(echo "$hhlst+$hrs" | bc)
  lstmm=$(echo "$mmlst+$mm" | bc | awk -F. '{print$1}')
  lstss=$(echo "$sslst+$ss" | bc | awk -F. '{print$1}')

  if [ $lsthh -ge 24 ]; then
    lsthh=$(echo "$lsthh-24" | bc)
  fi

  if [ $lstmm -ge 60 ]; then
    lsthh=$(echo "$lsthh+1" | bc)
    lstmm=$(echo "$lstmm-60" | bc)
  fi

  if [ $lstss -ge 60 ]; then
    lstmm=$(echo "$lstmm+1" | bc)
    lstss=$(echo "$lstss-60" | bc)
  fi

  if [ $lsthh -ge 0 ] && [ $lsthh -lt 10 ]; then
    lsthh=0$lsthh
  fi

  if [ $lstmm -ge 0 ] && [ $lstmm -lt 10 ]; then
    lstmm=0$lstmm
  fi

  if [ $lstss -ge 0 ] && [ $lstss -lt 10 ]; then
    lstss=0$lstss
  fi
}

function print_output() {
  echo " *******************************************"
  echo "          LST entered (hh:mm:ss): $lst"
  echo "   Fractional hours to add (hrs): $fhrs"
  echo "                  LST (hh:mm:ss): $lsthh:$lstmm:$lstss"
  echo " *******************************************"
  echo
}

if [ $# -lt 1 ]; then
  usage
  exit 1
fi

usage
format
print_output
