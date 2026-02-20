#!/bin/sh
#
# Show selected parameter in specified preset file
# Requres sed .
#
# Usage:
# sh showparam.sh <param-name> <.ttl-file>
#
# Example:
# sh setparam.sh unused_1 0.Mypatch.ttl
#

# get_param param-label file-name
get_param () {
  grep -A 1 $1 $2 | grep value | awk '{print $2}'
}

# show_param param-label file-name
show_param () {
  val=$(get_param $1 $2)
  [ -z "$val" ] && echo "Parameter $1 doesn't exist in $2" && exit 1
  echo "Parameter $1 is $val"
}

# Show selected parameter ($1) in file ($2)
show_param $1 $2
