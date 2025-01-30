#!/bin/sh
#
# Set selected parameter to specific value.
# Requres sed .
#
# Usage:
# sh setparam.sh <param-name> <value> <.ttl-file>
#
# Example:
# sh setparam.sh unused_1 0.0 Mypatch.ttl
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

# change_param param-label newval file-name
change_param () {
  val=$(get_param $1 $3)
  newval=$2
  echo "Parameter $1 is $val"
  echo "New value is $newval"
  set_param $1 $val $newval $3
}

# Change selected parameter ($1) to new value ($2) in file ($3)
change_param $1 $2 $3
