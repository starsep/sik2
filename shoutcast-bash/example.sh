#!/bin/bash
echo "GET / HTTP/1.0
Accept: */*
Icy-MetaData: 0
Connection: close
" > .tmp

timeout 10 nc stream3.polskieradio.pl 8904 < .tmp > tmp.mp3
mplayer tmp.mp3
