#!/bin/sh

SMALL_FILE="_144p"
BIG_FILE="_480p"
SRC_FILE=$(ls -l *.mp4 | awk '{print $9}')
for src in $SRC_FILE
do
	dst=$(ls -l $src | awk '{print substr($9, 1, index($9,".")-1)}')".flv"
	ffmpeg -i $src -c copy $dst
	echo $src"===>"$dst
done

SMALL_FILE_LIST=$(ls -l *$SMALL_FILE.flv | awk '{print $9}')
BIG_FILE_LIST=$(ls -l *$BIG_FILE.flv | awk '{print $9}')


SRC_FILE=$(ls -l *.flv | awk '{ if(match($9, "'"$SMALL_FILE"'")== 0 && match($9, "'"$BIG_FILE"'")==0) print $9}')
for src in $SRC_FILE
do
	find_flag=0
	dst=$(ls -l $src | awk '{print substr($9, 1, index($9,".")-1)}')"$SMALL_FILE.flv"
	for sfile in $SMALL_FILE_LIST
	do
		if [ $sfile = $dst ]; then
			find_flag=1
			break
		fi
	done
	if [ $find_flag -eq 0 ]; then
		ffmpeg -t 00:15  -i $src -y -profile:v baseline -c:v libx264 -aspect 192:144  -s 192x144 -an $dst
	  screen=$(ls -l $src | awk '{print substr($9, 1, index($9,".")-1)}')"$SMALL_FILE.jpg"
	  ffmpeg -i $dst -y  -r 1 -t 1  $screen
	  echo $src"===>"$dst
	fi
	
done

SRC_FILE=$(ls -l *.flv | awk '{ if(match($9, "'"$SMALL_FILE"'")== 0 && match($9, "'"$BIG_FILE"'")==0) print $9}')
for src in $SRC_FILE
do
	find_flag=0
	dst=$(ls -l $src | awk '{print substr($9, 1, index($9,".")-1)}')"$BIG_FILE.flv"
	for bfile in $BIG_FILE_LIST
	do
		if [ $bfile = $dst ]; then
			find_flag=1
			break
		fi
	done
	if [ $find_flag -eq 0 ]; then
		ffmpeg -t 30:00 -i $src -y -profile:v baseline -c:v libx264 -s 640x480 -c:a copy $dst
		echo $src"===>"$dst
	fi
	
done


