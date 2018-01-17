#!/usr/bin/env bash
UNIQUE_TAB_LINES_COUNT=$(sed 's/[^\t]//g' TEST-DATA.tsv | sort -u | wc -l)

if [ $UNIQUE_TAB_LINES_COUNT -eq "1" ]
then
  echo "All lines have the same number of tabs. All good!"
else
  echo "Not all lines have the same number of tabs."
  echo $UNIQUE_TAB_LINES_COUNT
  exit 1
fi
