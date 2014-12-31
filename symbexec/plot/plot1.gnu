#!/usr/bin/gnuplot
reset
fontsize = 11
set terminal pdf enhanced font "Verdana,5"
#set term postscript enhanced eps fontsize
set output "plot1.pdf"
set style fill solid 1.00 border 0
set style histogram errorbars
set style data histogram
set xtics rotate by -45
set grid ytics
set grid xtics
set xlabel "Tested application download websites"
set ylabel "Accuracy"
#set yrange [0:1]
set key below
#set ytics add("0" 0, "10%%" 0.1, "20%%" 0.2, "30%%" 0.3, "40%%" 0.4, "50%%" 0.5, "60%%" 0.6, "70%%" 0.7, "80%%" 0.8, "90%%" 0.9, "100%%" 1 )

set datafile separator ","
plot 'multi_apps_normalized_percentage_userinput_symbolic.dat' using 4:1:xtic(2) title "User Input Covered Basic Blocks" linecolor rgb 'red', \
	 'multi_apps_normalized_percentage_userinput_symbolic.dat' using 5:1 	     title "Symbolic Execution Covered Basic Blocks" linecolor rgb 'blue'
