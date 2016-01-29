#!/bin/bash
set -x

# gidoodl (formerly gifbutt) v612.1
# this lil guy makes is sickeasy to just make a thing
# on your screen into a gif. you need ffmpeg, and probably
# a specific version or something

temp_dir=~/tmp
output_dir=~/tmp
fps=25
id=$(date +%s)
temp_vid=$temp_dir/temp_${id}.mkv
log=$temp_dir/log_${id}.log

save_temp=1

# get capture rect from user
rect_string="$(./rect)"
wait $!
echo "$rect_string"

# split er up
IFS=',' read -a rect <<< "$rect_string"

# assign so hard
x=${rect[0]}
y=${rect[1]}
w=${rect[2]}
h=${rect[3]}

function die
{
    echo "${*} sucks."
    exit 1
}

# record
# TODO -capture_cursor -capture_mouse_clicks \
ffmpeg -framerate $fps \
    -video_size ${w}x$h \
    -f x11grab \
    -i :0.0+$x,$y \
    -an \
    -vcodec libx264 -crf 0 -preset ultrafast \
    $temp_vid &>> $log &

echo "\n\n" >> $log

if [ $! -eq 0 ]; then
    die "guess ffmpeg died."
fi

ffmpeg_pid=$!

read -p "Press any key to stop capture"

# TODO - if vid is too short, exit

kill $ffmpeg_pid

if [ $? -ne 0 ]; then
    echo "ffmpeg exited with $? :("
fi

# http://blog.pkh.me/p/21-high-quality-gif-with-ffmpeg.html
# TODO - make palette optimization optional
echo "generating palette. please hold onto all of your horses"
palette=$temp_dir/palette_${id}.png
filters="fps=$fps,scale=1366:-1:flags=lanczos"
ffmpeg -i $temp_vid \
    -vf "$filters,palettegen" \
    -y $palette &>> $log \
    || die "couldn't generate palette."

echo "\n\n" >> $log

echo "please, your horses. hold onto them"
ffmpeg -i $temp_vid -i $palette \
    -lavfi "$filters [x]; [x][1:v] paletteuse" \
    -y ${temp_vid}.gif &>> $log \
    || die "failed to create gif using generated palette."

echo "\n\n" >> $log

if [ -n $save_temp ]; then
    echo "cleanin up the mess i made earlier with all the hard work I was doing while you were just screwing around"
    rm $temp_vid $palette
fi

# TODO - rename and move gif

echo "all done. you did rill good"

# TODO - show gif filesize and duration details
