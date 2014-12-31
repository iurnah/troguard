#!/usr/bin/gnuplot

reset
fontsize = 11
set terminal pdf enhanced font "Verdana,5"
#set term postscript enhanced eps fontsize
set output "echo_code_coverage_vs_time.pdf"

set label 1 "set style line 1 lt 2 lc rgb \"red\" lw 3" at -0.4, -0.25, 0 left norotate back textcolor rgb "red"  nopoint offset character 0, 0, 0
set label 2 "set style line 2 lt 2 lc rgb \"orange\" lw 2" at -0.4, -0.35, 0 left norotate back textcolor rgb "orange"  nopoint offset character 0, 0, 0
set label 3 "set style line 3 lt 2 lc rgb \"yellow\" lw 3" at -0.4, -0.45, 0 left norotate back textcolor rgb "yellow"  nopoint offset character 0, 0, 0
set label 4 "set style line 4 lt 2 lc rgb \"green\" lw 2" at -0.4, -0.55, 0 left norotate back textcolor rgb "green"  nopoint offset character 0, 0, 0
set label 5 "plot ... lt 1 lc 3 " at -0.4, -0.65, 0 left norotate back textcolor lt 3 nopoint offset character 0, 0, 0
set label 6 "plot ... lt 3 lc 3 " at -0.4, -0.75, 0 left norotate back textcolor lt 3 nopoint offset character 0, 0, 0
set label 7 "plot ... lt 5 lc 3 " at -0.4, -0.85, 0 left norotate back textcolor lt 3 nopoint offset character 0, 0, 0
set style line 1  linetype 2 linecolor rgb "red"  linewidth 3.000 
set style line 2  linetype 2 linecolor rgb "orange"  linewidth 2.000 
set style line 3  linetype 2 linecolor rgb "yellow"  linewidth 3.000 
set style line 4  linetype 2 linecolor rgb "green"  linewidth 2.000 
# 
set style data linespoints
set xtics rotate by -45
set grid ytics
set grid xtics
set xlabel "Runing time of symbolic executation of echo"
set ylabel "Coverages"
set yrange [0:1.0]
#set key below
set ytics add("0" 0, "10%%" 0.1, "20%%" 0.2, "30%%" 0.3, "40%%" 0.4, "50%%" 0.5, "60%%" 0.6, "70%%" 0.7, "80%%" 0.8, "90%%" 0.9, "100%%" 1 )

set datafile separator ","
#
set title "Independent colors and dot/dash styles" 
unset colorbox
plot 'echo_coverages_time.dat' using 1:3:xtic(2) ls 1 title 'ls 1', \
	 'echo_coverages_time.dat' using 1:4  ls 2 title 'ls 2', \
	 'echo_coverages_time.dat' using 1:5  ls 3 title 'ls 3', \
	 'echo_coverages_time.dat' using 1:6  ls 4 title 'ls 4'


