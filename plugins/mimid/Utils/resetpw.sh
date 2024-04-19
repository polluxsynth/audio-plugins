#!/bin/sh
#
# Set pulse width to 0 for oscillators that are not using pulse width.
#
# Requires sed, dc and tr .
#
# Usage:
# sh resetpw.sh <.ttl-file>
#

# get_param param-label file-name
get_param () {
  grep -A 1 $1 $2 | grep value | awk '{print $2}'
}

# set_param param-label old-value new-value file-name
set_param () {
  #echo set_param $1 $2 $3 $4
  #echo "sed -i /$1/{n;s/$2/$3/}" $4
  sed -i "/$1/{n;s/$2/$3/}" $4
}

# Obtain osc waveform, if it is not 2.0 (= Pulse)
# set PW to 0.
#
# Do same for both oscs

osc1wave=$(get_param osc1wave $1)
echo "osc 1 wave is $osc1wave"
if [ $osc1wave != 2.0 ]; then
  osc1pw=$(get_param osc1pw $1)
  echo "osc 1 pw was $osc1pw; setting to 0.0"
  set_param osc1pw $osc1pw 0.0 $1
fi

osc2wave=$(get_param osc2wave $1)
echo "osc 2 wave is $osc2wave"
if [ $osc2wave != 2.0 ]; then
  osc2pw=$(get_param osc2pw $1)
  echo "osc 2 pw was $osc2pw; setting to 0.0"
  set_param osc2pw $osc2pw 0.0 $1
fi
