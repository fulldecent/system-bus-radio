#!/bin/bash

# Ensure every line has the same number of tabs

HISTOGRAM=$(cat TEST-DATA.tsv | tr -cd '\t\n' | tr '\t' '.' | sed '${/^$/d;}' | sort | uniq -c)
echo $HISTOGRAM

if [ $(echo $HISTOGRAM | wc -w) -eq 2 ]; then
  echo "✅ All lines have the same number of tabs"
else
  echo "❌ Not all lines have the same number of tabs"
  exit 1
fi