#!/bin/sh

set -e

description=$1
if [ -n "$description" ]; then
	description="_$description"
fi

dir="backup/`date +%Y-%m-%d_%H-%M-%S`$description"
mkdir -p $dir
cp robocup-* $dir/
cp settings.xml $dir/
