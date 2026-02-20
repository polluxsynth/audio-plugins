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

# Not critical, but the parameters are saved in alphabetical order,
# so add new parameters in the same vain.

add_param "\"unisondet\"" "\"tune\"" 0.0 $1
add_param "\"unisonwidth\"" "\"unisondet\"" 10.0 $1

add_param "\"modwamt\"" "\"lfospread\"" 0.0 $1
add_param "\"modwdest\"" "\"modwamt\"" 0.0 $1
add_param "\"atamt\"" "\"aftertouchscale\"" 0.0 $1
add_param "\"atdest\"" "\"atamt\"" 0.0 $1

add_param "\"lfo3amount\"" "\"lfo2wave\"" 0.0 $1
add_param "\"lfo3amtcont\"" "\"lfo3amount\"" 0.0 $1
add_param "\"lfo3contramt\"" "\"lfo3amtcont\"" 0.0 $1
add_param "\"lfo3dest\"" "\"lfo3contramt\"" 0.0 $1
add_param "\"lfo3polarity\"" "\"lfo3dest\"" 0.0 $1
add_param "\"lfo3rate\"" "\"lfo3polarity\"" 6.0 $1
add_param "\"lfo3shape\"" "\"lfo3rate\"" 2.5 $1
add_param "\"lfo3sync\"" "\"lfo3shape\"" 0.0 $1
