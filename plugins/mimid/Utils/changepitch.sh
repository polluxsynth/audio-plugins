#!/bin/sh
#
# Shift oscillator pitches by specified amount
# Requres sed, dc and tr .
#
# Usage:
# sh changepitch.sh <shift> <.ttl-file>
#
# Example:
# sh changepitch.sh 12.0 Mypatch.ttl
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

# change_param param-label delta file-name
change_param () {
  val=$(get_param $1 $3)
  delta=$(echo $2 | tr '-' '_')
  echo "Parameter $1 is $val"
  newval=$(dc -e "$val $delta + p")
  echo "New value is $newval"
  set_param $1 $val $newval $3
}

# Increase osc pitch by supplied amount
change_param osc1pitch $1 $2
change_param osc2pitch $1 $2
