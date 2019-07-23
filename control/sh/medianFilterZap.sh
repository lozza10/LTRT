#!/bin/bash
#
# medianFilterZap.sh
# Impulsive RFI excision by a running median filter
#
# Version 1.03
# LT 06/13
#
# Notes: Window of 71 seems adequate
#
  
infile=$1
window=$2
lststart=$3
datestart=$4

prefix=`basename $infile .dat`
outfile="$prefix.zp"
outimg="$outfile.png"
wmid=`echo "$window/2" | bc -l  | xargs printf "%1.0f"` #round up
cnt=2
IFS=$OLDIFS

if [ $# != 4 ]; then
  echo
  echo -e " > median_filter_zap.sh <\n"
  echo -e " Usage: median_filter_zap.sh <file to zap> <median filter window size> <LST start (hh:mm:ss)> <start date (dd:mm:yyyy)>\n"
  echo -e " No arguments entered. Script must be run from working directory.\n"
  echo
  exit
fi

cat $infile | awk '!/^#/ {print}' > datfile.tmp
nlines=`cat datfile.tmp | wc | awk '{print$1}'`
wmaxline=`echo "$nlines-$wmid" | bc`

if [ -f $outfile ]; then
  rm $outfile
fi

if [ -f dat.tmp ]; then
  rm dat.tmp
fi

if [ -f sorted.tmp ]; then
  rm sorted.tmp
fi

function plot() {
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
}

# main loop
while read line
  do

    if [ $cnt -lt $wmid ]; then
      minwin=`echo "$cnt-1" | bc`
      min=`echo "$cnt-$minwin" | bc`
      mid=$cnt
      max=`echo "$cnt+$minwin" | bc`
      midlst=`awk 'NR=='$mid' {print$1}' datfile.tmp`
      medianline=$minwin
    elif [ $cnt -gt $wmaxline ]; then
      minwin=`echo "$nlines-$cnt" | bc`
      min=`echo "$cnt-$minwin" | bc`
      mid=$cnt
      max=`echo "$cnt+$minwin" | bc`
      midlst=`awk 'NR=='$mid' {print$1}' datfile.tmp`
      medianline=$minwin
    else
      minwin=`echo "$window/2" | bc | xargs printf "%1.0f"`
      min=`echo "$cnt-$minwin" | bc`
      mid=$cnt
      max=`echo "$cnt+$minwin" | bc`
      midlst=`awk 'NR=='$mid' {print$1}' datfile.tmp`
      medianline=$minwin
    fi
    touch dat.tmp sorted.tmp

    # build array of n window size 
    for ((n=$min; n<=$max; n++))
      do
        windowline=`awk 'NR=='$n' {print}' datfile.tmp`
        echo $windowline >> dat.tmp
      done

    cat dat.tmp | sort -n -k2 > sorted.tmp
    readarray -t array < sorted.tmp

    median=`echo "${array[$medianline]}" | awk '{print$2}'`
    temp=`echo "${array[$medianline]}" | awk '{print$3}'`
    echo "$midlst $median $temp" >> $outfile

    rm dat.tmp sorted.tmp

    if [ $cnt -eq `echo "$nlines-1" | bc` ]; then
      plot
      rm datfile.tmp
      exit
    fi

    cnt=`echo "$cnt+1" | bc`

  done < datfile.tmp
