#!/bin/bash
range=$(seq 1 30)

for image in "fruits.jpg" "green-eye.jpg"; do
    for radius in $range; do
        perf stat \
            --repeat 5 \
            --output bench-$image.log \
            --append \
            --detailed \
            ./fast_median_filter --radius $radius --target out.jpg $image
    done;
done;