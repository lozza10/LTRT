#!/bin/bash
#
# smoothData.sh
# Smooths LTRT ADC output and calls plotData.sh to plot out
#
# Version: 1.04
# LT 01/14
#

data_dir=$LTRT_DATA

# Start png viewer
gthumb -s &

# Append the LST
function append_lst() {
  j=$1
  lst=$2
  tsecs=$3
  ampl=$4
  normampl=$5
  temp=$6

  lsthhstr=$(echo "$lst" | awk -F: '{print$1}')
  lstmmstr=$(echo "$lst" | awk -F: '{print$2}')
  lstssstr=$(echo "$lst" | awk -F: '{print$3}')
  hh=$(echo "$tsecs/3600" | bc)
  mm=$(echo "($tsecs%3600)/60" | bc)
  ss=$(echo "$tsecs%60" | bc)
  lsthh=$(echo "$lsthhstr+$hh" | bc)
  lstmm=$(echo "$lstmmstr+$mm" | bc)
  lstss=$(echo "$lstssstr+$ss" | bc)

  chk=`echo $lstss'>'59 | bc`
  if [ $chk -eq 1 ]; then
   lstmm=$(echo "$lstmm+1" | bc)
   lstss=$(echo "$lstss-60" | bc)
  fi

  chk2=`echo $lstss'<'1 | bc`
  chk3=`echo $lstss'>='1 | bc`
  chk4=`echo $lstss'<'10 | bc`
  if [ $chk3 -eq 1 ] && [ $chk4 -eq 1 ]; then
    lstss=0$lstss
    else if [ $chk2 -eq 1 ]; then
      #lstss=00$lstss
      lstss=00 #temp
    fi
  fi

  if [ $lstmm -gt 59 ]; then
   lsthh=$(echo "$lsthh+1" | bc)
   lstmm=$(echo "$lstmm-60" | bc)
  fi

  if [ $lstmm -ge 0 ] && [ $lstmm -lt 10 ]; then
    lstmm=0$lstmm
  fi

  if [ $lsthh -ge 24 ]; then
   lsthh=$(echo "$lsthh-24" | bc)
  fi

  if [ $lsthh -ge 0 ] && [ $lsthh -lt 10 ]; then
   lsthh=0$lsthh
  fi

  # output format: [d-hh:mm:ss float float] where d is indexed day
  echo "$j-$lsthh:$lstmm:$lstss $ampl $normampl $temp" >> $hdrfile
}

# Main loop

while [ 1 -lt 2 ]
  do
    j=1
    lstss=0

    # get newest file with .tmp ext
    indir=`find $data_dir -type d -exec stat --printf="%n\n" {} \; | sort -n | tail -1`
    infile=`ls -t $indir/*.tmp | head -1`

    # check status of analogueCapture script
    acstatus=`ps -C analogueCapture | awk '{if (NR==2) {print}}' | wc -l`

    if [ "$infile" == "" ] || [ $acstatus -eq 0 ]; then
      echo -e " smoothData.sh :: `date +%X` :: Waiting for new file..."
    else
      # set vars
      prefix=`basename $infile .tmp`
      outfile="$indir/$prefix.dat"
      outimg="$indir/$prefix.png"
      hdrfile="$indir/$prefix.hdr"

      # get val and temp starts by averaging over first n lines
      valstart=`cat $infile | awk 'NF' | awk 'NR<5000 {sum+=$2}END{print sum/5000}'`

      # create header file      
      touch $hdrfile

      # extract header information
      OLDIFS=$IFS
      IFS=","
      hdr_array=($(cat $outfile | awk '{OFS=","} /^#/ {print}'))
      hdr_elems=${#hdr_array[@]}

      for ((i=0; i<$hdr_elems; i++))
        do
          echo ${hdr_array[$i]} >> $hdrfile
          lst_start=`echo ${hdr_array[$i]} | grep "LST_start" | awk '{print$3}'`
          filename=`echo ${hdr_array[$i]} | grep "File" | awk '{print$3}'`
        done

      # smooth the data
      echo -e "\n\n\n"
      echo " smoothData.sh :: `date +%X` :: Smoothing $infile..."
      IFS=$OLDIFS
      array=($(awk 'NF == 3' $infile | awk 'NF' | awk '{OFS=","} {sum+=$2} (NR%5000)==0{print $1/1000,sum/5000,$3/2; sum=0;}'))
      elems=${#array[@]}

      for ((i=0; i<$elems; i++))
        do
           tsecs=$(echo ${array[$i]} | awk -F, '{printf "%.1f", $1}')
           ampl=$(echo ${array[$i]} | awk -F, '{print$2}')
           temp_current=$(echo ${array[$i]} | awk -F, '{print$3*2}')
           #temp_current=$(echo ${array[$i]} | awk -F, '{print$3}')
           temp_current_c=$(echo "scale=3;$temp_current/16.0" | bc)
           normampl=$(echo "scale=3;$ampl/$temp_current_c" | bc)

           # this hack allows gnuplot to plot without wrapping the LST
           chk5=`echo $lstss'>='53 | bc`
           if [ "$lsthh:$lstmm" == "23:59" ] && [ $chk5 -eq 1 ]; then
             j=$(echo "$j+1" | bc)
             k=$j
           else
             k=$j
           fi
           append_lst $k $lst_start $tsecs $ampl $normampl $temp_current_c
        done

      # print stats to stdout 
      echo " ------------------------------------------------------------------"
      OLDIFS=$IFS
      IFS=","
      for ((i=0; i<$hdr_elems; i++))
        do
          echo ${hdr_array[$i]}
        done
      IFS=$OLDIFS
      echo " ------------------------------------------------------------------"
      echo "Noise amplitude at obs start [arbitary]: $valstart"
      echo "Current feed temperature [Degrees Celsius]: $temp_current_c" #temp jan 14
      echo " ------------------------------------------------------------------"

      # copy back to outfile
      cat $hdrfile > $outfile

      # plot
      ./plotData.sh $outfile $outimg $filename
      
      # clean up
      rm $hdrfile

    fi

    echo -e " smoothData.sh :: `date +%X` :: Sleeping...\n"
    sleep 1

  done
