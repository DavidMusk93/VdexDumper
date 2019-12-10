#!/bin/bash

#set -x

case $1 in
  0|find_secure)
    v=`grep "#[[:blank:]]*define[[:blank:]]SECURE_DIR" *.h`
    v=${v#*\"}
    echo ${v%\"*}
    ;;
esac
