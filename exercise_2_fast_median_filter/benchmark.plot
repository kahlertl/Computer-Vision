set xlabel "filter radius (px)"
set ylabel "time (seconds)"

# Regression function
f(x) = a*x + b

# Output format and file
set term png
set output "benchmark.png"

set multiplot layout 2,1 title "Huang's histogram based median filter"

set xrange [1:30]

fit f(x) "fruits-1.dat" using 1:2 via a, b
set title "512x480px"
set ytics 0.2
plot "fruits-1.dat" using 1:2 title "" with points ls 1, \
     f(x) ls 1 title ""


fit f(x) "green-eye-1.dat" using 1:2 via a, b
set title "1920x1200px"
set ytics 2
plot "green-eye-1.dat" using 1:2 title "" with points ls 1, \
     f(x) ls 1 title ""
