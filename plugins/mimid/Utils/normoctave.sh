#!/bin/sh
#
# Normalize octave setting to 0, by adjusting the oscillator
# pitches and filter frequency accordingly.
#
# Requires sed, dc and tr .
#
# Usage:
# sh normoctave.sh <.ttl-file>
#

# get_param param-label file-name
get_param () {
  grep -A 1 $1 $2 | grep value | awk '{print $2}'
}

# set_param param-label old-value new-value file-name
set_param () {
  #echo set_param $1 $2 $3 $4
  #echo "/$1/{n;s/$2/$3/}" $4
  sed -i "/$1/{n;s/$2/$3/}" $4
}

# Normalize octave to be 0.0
# tr '-' '_' converts potential negative numbers to dc compatible format

octave=$(get_param octave $1)
oct=$(echo $octave | tr '-' '_')
shift=$(dc -e "0 $oct - p" | tr '-' '_')
newoctave=$(dc -e "$oct $shift + p")
newoctave="${newoctave}"
if [ $newoctave -ne 0 ]; then
  echo "$1: Octave didn't normalize"
fi
echo octave $octave $shift $newoctave

# change sign of shift for oscpitch
shift=$(dc -e "0 $shift - p" | tr '-' '_')
pitchshift=$(dc -e "12 $shift * p" | tr '-' '_')

pitch1=$(get_param osc1pitch $1)
newpitch1=$(dc -e "$pitch1 $pitchshift + p")
echo osc1pitch $pitch1 $pitchshift $newpitch1

pitch2=$(get_param osc2pitch $1)
newpitch2=$(dc -e "$pitch2 $pitchshift + p")
echo osc2pitch $pitch2 $pitchshift $newpitch2

# When we change the glbal octave, we change the filter by the tracking amount,
# so compensate for that
cutoff=$(get_param cutoff $1)
keytrack=$(get_param keytrack $1)
cshift=$(dc -e "$keytrack $shift * p" | tr '-' '_')
newcutoff=$(dc -e "$cutoff $cshift + p")
echo "cutoff $cutoff $newcutoff (keytrack $keytrack)"

set_param octave $octave 0.0 $1
set_param osc1pitch $pitch1 $newpitch1 $1
set_param osc2pitch $pitch2 $newpitch2 $1
set_param cutoff $cutoff $newcutoff $1
