#!/usr/bin/gnuplot

reset
set terminal pdf size 10,5 enhanced font 'Verdana,9'
set output 'recall.pdf'

set boxwidth 1
set style data histogram
set style histogram cluster gap 5

set style fill solid border rgb "black"
set auto x

set grid ytics
set grid xtics
set key below
set yrange [0:*]

set ytics add("0" 0, "10%%" 0.1, "20%%" 0.2, "30%%" 0.3, "40%%" 0.4, "50%%" 0.5, "60%%" 0.6, "70%%" 0.7, "80%%" 0.8, "90%%" 0.9, "100%%" 1 )
set xlabel "Different Attributes Domains"
set ylabel "Recall"

plot 'recall_plot.data' using 2:xtic(1) title col lc rgb "#FF0000",\
        '' using 3:xtic(1) title col lc rgb "#3333FF", \
        '' using 4:xtic(1) title col lc rgb "#FF007F", \
        '' using 5:xtic(1) title col lc rgb "#FF8000", \
        '' using 6:xtic(1) title col lc rgb "#7F00FF", \
        '' using 7:xtic(1) title col lc rgb "#FFFF00", \
        '' using 8:xtic(1) title col lc rgb "#0080FF", \
        '' using 9:xtic(1) title col lc rgb "#C0C0C0", \
        '' using 10:xtic(1) title col lc rgb "#8B4513", \
        '' using 11:xtic(1) title col lc rgb "#00FFFF", \
        '' using 12:xtic(1) title col lc rgb "#000000"
