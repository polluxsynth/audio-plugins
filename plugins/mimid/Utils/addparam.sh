#!/bin/sh
#
# Add new parameters to preset files
# Requres awk .
# Needs to be modified depending on parameters to be added
#
# Usage:
# sh addparam.sh <file.ttl>
#
# Example:
# sh addparam.sh Mypatch.ttl
#

# add_param param-name prev-param-name default-value file-name
add_param() {
  echo "search for parameter $1 in file $4"
  echo "if not found add parameter $1 after $2 with default value $3"
  #sed -i "/$1/{n;r tmp}" $4
  # Skip if parameter already present
  grep $1 $4 > /dev/null && return
  newparam="\
	] , [\n\
		lv2:symbol \\\"\"$1\"\\\" ;\n\
		pset:value $3"
  #echo "Adding $newparam"
  awk "1;/$2/{c=2}c&&!--c{print \"$newparam\"}" $4 > tmp
  [ $? -eq 0 ] && mv tmp $4
  rm -f tmp
}

add_param "\"hold\"" "\"filterspread\"" 0.0 $1
add_param "\"filterhold\"" "\"filterenvamount\"" 0.0 $1
