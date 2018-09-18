#!/bin/bash

SMALL_FILE="_144p"
BIG_FILE="_480p"

SMALL_FILE_LIST=$(ls -l *$SMALL_FILE.flv | awk '{print $9}')
BIG_FILE_LIST=$(ls -l *$BIG_FILE.flv | awk '{print $9}')

id=0;
SERVER="http://192.168.200.100:9980/"
echo -e "{\"list\":[ \c" > list.data
for file in $SMALL_FILE_LIST
do
	highvideo=$(ls -l $file | awk '{print substr($9, 1, index($9,"'"$SMALL_FILE"'")-1)}')$BIG_FILE".flv"
	image=$(ls -l $file | awk '{print substr($9, 1, index($9,".")-1)}')".jpg"
	#image="miya_144p.jpg"
	#file="1_hah.flv"
	id=`expr $id + 1`;
	count=`expr $id \* 10`;
	echo -e "{\"id\":\"$id\", \c" >> list.data
	echo -e "\"title\":\"����$file\", \c" >> list.data
	echo -e "\"auth\":\"����$file\", \c" >> list.data
	echo -e "\"people\":\"$count.0��\", \c" >> list.data
	echo -e "\"videourl\":\"$SERVER$file\", \c" >> list.data
	echo -e "\"imageurl\":\"$SERVER$image\", \c" >> list.data
	echo -e "\"highvideourl\":\"$SERVER$highvideo\" }, \c" >> list.data
done

echo -e "{\"id\":\"0\", \c" >> list.data
echo -e "\"videourl\":\"\", \c" >> list.data
echo -e "\"imageurl\":\"\", \c" >> list.data
echo -e "\"highvideourl\":\"\" } \c" >> list.data
echo -e " ]}" >> list.data





