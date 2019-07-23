#!/bin/bash
#
# plotData.sh 
# Version 1.04
# Plots LTRT ADC output to png
# 
# LT 04/14
#

datfile=$1
outimg=$2
filename=$3

echo -e "   plotData.sh :: `date +%X` :: Plotting..."

if [ -f $datfile ]; then
gnuplot <<EOF
# defaults
set term png x202020 size 1250, 450 font '/usr/share/fonts/liberation/LiberationSans-Regular.ttf' 10
set out "$outimg"
set offsets
set autoscale fix
set size 1,1
set nokey
set xlabel ""

# line styles
set style line 1 lt 1 lw 0.5 linecolor rgb "white"
set style line 2 lt 1 lw 0.5 linecolor rgb "green"
set style line 3 lt 1 lw 0.2 linecolor rgb "gray"
set style line 4 lt 1 lw 0.5 linecolor rgb "red"

# style defaults
set border ls 3
set key textcolor ls 3

# x axis formats
set xdata time
set timefmt "%d-%H:%M:%S"

# multiplot
set multiplot layout 3, 1 rowsfirst
set lmargin at screen 0.1
set rmargin at screen 0.96
set title "LTRT 3700MHz Drift Scan :: $filename" tc ls 3

## top plot (amplitude)
set format x ""
set tmargin at screen 0.88
set bmargin at screen 0.64
set ylabel "Amplitude (arb)" tc ls 3
plot "$datfile" u 1:2 w lines ls 2 title "Digitised voltage"

## middle plot (amplitude normalised by feed temperature)
unset title
set format x ""
set tmargin at screen 0.62
set bmargin at screen 0.38
set ylabel "Amplitude (arb)" tc ls 3
plot "$datfile" u 1:3 w lines ls 4 title "Digitised voltage (normalised)"

## lower plot (feed temperature)
set format x "%H:%M"
set tmargin at screen 0.36
set bmargin at screen 0.12
set ylabel "Degrees C" tc ls 3
set xlabel "LST (hh:mm)" tc ls 3
plot "$datfile" u 1:4 w lines lt 0 lc 4 title "Feed temperature"
unset multiplot
EOF
else
  echo "$datfile does not exist!"
fi

