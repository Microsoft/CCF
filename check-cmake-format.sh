#!/bin/bash
# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the Apache 2.0 License.

set -u

if [ "$#" -eq 0 ]; then
  echo "No args given - specify dir(s) to be formatted"
  exit 1
fi

fix=false
while getopts ":f:" opt; do
  case $opt in
    f)
      fix=true
      shift
    ;;
    \?) echo "Invalid option -$OPTARG" >&2
      exit
    ;;
  esac
done

if $fix ; then
  echo "Formatting files in" "$@"
else
  echo "Checking file format in" "$@"
fi

if [ ! -f "env/bin/activate" ]
    then
        python3.7 -m venv env
fi

source env/bin/activate
pip install cmake_format

unformatted_files=""
for file in $(find "$@" -name "*.cmake"); do
  cmake-format --check "$file"
  d=$?
  if $fix ; then
    cmake-format -i "$file"
  fi
  if [ $d -ne 0 ]; then
    if [ "$unformatted_files" != "" ]; then
      unformatted_files+=$'\n'
    fi
    unformatted_files+="$file"
  fi
done

if [ "$unformatted_files" != "" ]; then
  echo "Fix formatting:"
  echo "$unformatted_files"
  exit 1
else
  echo "All files formatted correctly!"
fi