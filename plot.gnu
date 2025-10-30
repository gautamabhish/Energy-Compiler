set terminal pngcairo size 1000,600 enhanced font 'Verdana,13'
set output 'bench.png'

set title "Energy-Aware Compiler vs Baseline" font ',16'
set grid ytics lc rgb "#cccccc" lw 1 lt 0

set style data histograms
set style histogram cluster gap 1
set style fill solid 0.8 border -1
set boxwidth 0.8
set border lw 1.5

set key outside top center box opaque
set object 1 rectangle from screen 0,0 to screen 1,1 fillcolor rgb"#fafafa" behind

set ylabel "Execution Time (ms)" textcolor rgb "#4CAF50"
set y2label "Energy (J)" textcolor rgb "#2196F3"
set ytics nomirror
set y2tics nomirror

set xtics font ',13'
set ytics font ',12'
set y2tics font ',12'
set yrange [0:0.5]          # Time axis up to 0.05 ms
set y2range [0:0.01]         # Energy axis up to 0.01 J
# Data format:
# Mode  Time(ms)  Energy(J)
# Baseline 0.36272 0.009068
# EnergyAware 0.1 0.003968

plot 'data.dat' using 2:xtic(1) title 'Execution Time (ms)' lc rgb "#2196F3"  axes x1y1, \
     '' using 3 title 'Energy (J)' lc rgb "#4CAF50" axes x1y2
