#!/usr/bin/gnuplot
reset
#fontsize = 11
set terminal pdf enhanced font "Verdana,6"
#set terminal pdf enhanced font "Verdana,5"
#set term postscript enhanced eps fontsize
#set output "web_analysis_accuracy.pdf"
set output "web_analysis_accuracy_shrink.pdf"
set style fill solid 1.00 border 0
set style histogram errorbars
set style data histogram
set xtics rotate by -45
set grid ytics
set grid xtics
#set xlabel "Tested application download websites"
set ylabel "Accuracy"
set yrange [0:1.01]
set key inside top right
#set nokey
set ytics add("0" 0, "10%%" 0.1, "20%%" 0.2, "30%%" 0.3, "40%%" 0.4, "50%%" 0.5, "60%%" 0.6, "70%%" 0.7, "80%%" 0.8, "90%%" 0.9, "100%%" 1 )
set datafile separator ","

plot 'website_analysis_results_percentage_sorted.dat' using 2:3:xtic(1) ti "Text analysis" linecolor rgb "blue", \
	 'website_analysis_results_percentage_sorted.dat' using 4:5 ti "OCR analysis" lt 1 lc rgb "red"
