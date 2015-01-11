#!/bin/bash

[ ! -d 'imgs' ] && mkdir imgs

for height in $(seq 100 100 1200); do
    convert green-eye.jpg -resize x$height imgs/green-eye-$height.jpg
done