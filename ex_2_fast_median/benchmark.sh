#!/bin/bash

[ ! -d "stats" ] && mkdir stats

for image in "fruits.jpg" "green-eye.jpg"; do
    for radius in $(seq 1 30); do
        perf stat \
            --repeat 5 \
            --output bench-$image.log \
            --append \
            --detailed \
            ./fast_median_filter --radius $radius --target out.jpg $image
    done
done

echo "# Show linar time complexity in the image size" > stats/resize.log
for image in imgs/*.jpg; do
    perf stat \
        --repeat 5 \
        --output stats/resize.log \
        --append \
        --detailed \
        ./fast_median_filter --radius 3 --target out.jpg $image
done