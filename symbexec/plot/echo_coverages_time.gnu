#!/usr/bin/gnuplot

reset
set terminal pdf enhanced font "Verdana,7"
#set terminal pngcairo dashed
#set term postscript enhanced eps fontsize
#set output "echo_coverages_time.pdf"
set output "echo_coverages_time_normalized_percentage_shift.pdf"
#set output "echo_coverages_time_normalized_percentage.pdf"
#set output "echo_coverages_time.png"
set style data linespoints
set xtics rotate by -45
set grid ytics
set grid xtics
#set xlabel "Runing time of symbolic executation of echo"
set ylabel "Normalized Code Coverages"
set yrange [0:1.01]
set key inside right bottom
#set key inside left top
set ytics add("0" 0, "10%%" 0.1, "20%%" 0.2, "30%%" 0.3, "40%%" 0.4, "50%%" 0.5, "60%%" 0.6, "70%%" 0.7, "80%%" 0.8, "90%%" 0.9, "100%%" 1 )

set datafile separator ","
#
# define line styles using explicit rgbcolor names
#
set style line 1  lt 2 linecolor rgb "red"  linewidth 4.000 
set style line 2  lt 7 linecolor rgb "black"  linewidth 5.000 
set style line 3  lt 12 linecolor rgb "green"  linewidth 6.000 
set style line 4  lt 22 linecolor rgb "blue"  linewidth 7.000 
#

#plot 'echo_coverages_time_normalized_percentage.dat' using 1:3:xtic(2) title "Basic Block Coverage", \
#     'echo_coverages_time_normalized_percentage.dat' using 1:4 		 title "Function Block Coverage",  \
#	 'echo_coverages_time_normalized_percentage.dat' using 1:5 		 title "Total Touched Functions",  \
#	 'echo_coverages_time_normalized_percentage.dat' using 1:6 		 title "Fully Covered Functions"

plot 'echo_coverages_time_normalized_percentage_shift.dat' using 1:3:xtic(2) title "Basic Block Coverage", \
     'echo_coverages_time_normalized_percentage_shift.dat' using 1:4 		 title "Function Block Coverage",  \
	 'echo_coverages_time_normalized_percentage_shift.dat' using 1:5 		 title "Total Touched Functions",  \
	 'echo_coverages_time_normalized_percentage_shift.dat' using 1:6 		 title "Fully Covered Functions"
	 
#plot 'echo_coverages_time.dat' using 1:3:xtic(2) title "Basic Block Coverage", \
#     'echo_coverages_time.dat' using 1:4 		 title "Function Block Coverage",  \
#	 'echo_coverages_time.dat' using 1:5 		 title "Total Touched Functions",  \
#	 'echo_coverages_time.dat' using 1:6 		 title "Fully Covered Functions"
