#!/bin/sh
if test ! -f "$1"
then
 echo "Error: executable $1 does not exist."
 exit 1
fi
if test ! -f "$2"
then
 echo "Error: trace log $2 does not exist."
 exit 1
fi
EXECUTABLE="$1"
TRACELOG="$2"
while read LINETYPE FADDR CADDR; do
 FNAME="$(addr2line -f -e ${EXECUTABLE} ${FADDR}|head -1)"
 if test "${LINETYPE}" = "enter"
 then
 CNAME="$(addr2line -f -e ${EXECUTABLE} ${CADDR}|head -1)"
 CLINE="$(addr2line -s -e ${EXECUTABLE} ${CADDR})"
 echo "Enter ${FNAME} called from ${CNAME} (${CLINE})"
 fi
 if test "${LINETYPE}" = "exit"
 then
 echo "Exit ${FNAME}"
 fi
done < "${TRACELOG}"
