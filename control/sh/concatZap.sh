#!/bin/bash
#
# concatZap.sh
# LTRT data file concatenation, and impulsive RFI excision
# by median_filter_zap.sh
#
# Version 1.03
# LT 06/13
#
# Notes: Window of 71 seems adequate
#

data_dir=$1
outimg="$data_dir/$2.png"
window=$3
cnt=0

if [ $# != 3 ]; then
  echo
  echo -e " > concatZap.sh <\n"
  echo -e " LTRT data file concatenation, and impulsive RFI excision\n"
  echo -e " Usage: concatZap.sh <path to data directory> <concatenated output image prefix> <filter window size (odd numbers only)>\n"
  echo -e " You must create a datlist first, containing all .dat files! Please run <ll -rt *.dat | awk '{print dollar9}'> and edit\n"
  exit
fi
 
  cd $data_dir

  if [ -f *concat.dat ]; then
    rm *concat.dat
  fi

  # Comment out for manually edited datlist
  #ls -rt *.dat > datlist

  if [ ! -f datlist ]; then
    echo "No datlist found. Please run <ll -rt *.dat | awk '{print dollar9}'> and edit"
    exit
  fi
   
  echo
  echo "Concatenating files..."

  for line in $(<datlist)
    do
      echo "  $line"
      if [ $cnt -eq 0 ]; then
        prefix=`basename $line .dat`
        outfile="$prefix-concat.dat"

        # extract header information
        OLDIFS=$IFS
        IFS=","
        hdr_array=($(cat $line | awk '{OFS=","} /^#/ {print}'))
        hdr_elems=${#hdr_array[@]}

        for ((i=0; i<$hdr_elems; i++))
          do
            lst_start=`echo ${hdr_array[$i]} | grep "LST_start" | awk '{print$3}'`
            date=`echo ${hdr_array[$i]} | grep "LocalTime_start" | awk '{print$4}'`
          done

        awk '{print}' $line > $outfile
      else 
        lastline=`awk 'END{print}' $outfile`
        d=$(echo $lastline | awk -F- '{print$1}')
        if [ $d -ne 1 ]; then
          awk '!/^#/ {sub(/1/,'$d');print}' $line >> $outfile
        else
          awk '!/^#/ {print}' $line >> $outfile
        fi 
      fi
      cnt=$((cnt+1));
    done

# plot
gnuplot <<EOF
set term png size 1250, 400 font '/usr/share/fonts/liberation/LiberationSans-Regular.ttf' 10
set out "$outimg"
set title "$outfile" tc ls 1
set style line 1 lt 1 lw 0.5 linecolor rgb "white"
set style line 2 lt 1 lw 0.5 linecolor rgb "green"
set border ls 1
set key textcolor ls 1
set xlabel "LST (hh:mm)" tc ls 1
set ylabel "Amplitude (arbitary)" tc ls 1
set obj 1 rectangle from screen 0,0 to screen 1,1 fillcolor rgbcolor "black" behind
set xdata time
set timefmt "%d-%H:%M:%S"
set format x "%H:%M"
plot "$outfile" u 1:2 w lines ls 2 title "Digitised voltage", "$outfile" u 1:3 w lines lt 0 lc 4 title "Feed temperature"
EOF

  echo
  echo "Zapping files..."
 
  #for line in $(<datlist)
  while read line
    do
      echo "  Working on file $line..."
      medianFilterZap.sh $line $window $lst_start $date
    done < datlist

  # check integrity
  if [ -f $outfile ] && [ -f $outimg ] && [ `cat $outfile | wc -l` -gt 0 ] ; then
    echo
    echo -e "Complete.\n"
  else
    echo
    echo -e "Write failed!\n"
  fi

  cd -
