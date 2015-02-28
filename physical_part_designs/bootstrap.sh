#!/usr/bin/env bash

DIR="$( cd "$( dirname "$0" )" && pwd )"
cmdfail="echo -ne \e[0;41;97mFAIL: "

set -e
set -u


echo downloading dependencies
while read line ; do
  [[ "$line" =~ ^\ *#.*$ ]] && echo skip $line && continue
  fname="./external/${line%-->*}"
  url="${line#*-->}"

  echo
  echo "GET $url"
  echo "WRITE $fname"
  mkdir -p "`dirname \"$fname\"`"
  cmdwget="wget -O "$fname" "$url""
  $cmdwget 2>/dev/null 1>/dev/null || {
    $cmdfail
    $cmdwget
  }
done < "$DIR/dependencies.txt"
echo successfully fetched dependencies


{
  echo -e "\n====="
  echo "OPENSCADPATH should be set in your OS environment"\
   "so downloaded files are available to openscad."
  echo "I suggest adding one of these two lines to your ~/.bashrc:"
  echo -en "\n\e[0;1;40;33m"
  echo "  export OPENSCADPATH=$DIR/external:\$OPENSCADPATH"
  echo "OR"
  echo "  export OPENSCADPATH=./external:\$OPENSCADPATH"
  exit 1
}

