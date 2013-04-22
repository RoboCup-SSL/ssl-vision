#!/bin/sh
for filename in *.svg
do
  filenameshort=$(basename $filename .svg)
  #note that according to the docs. +antialias should *disable* antialiasing which is what we desire
  convert -density 100 $filename +dither -stroke white -colors 32 +antialias -filter point -sample 800x600! -despeckle $filenameshort.png
done;

