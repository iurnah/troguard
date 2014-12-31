#!/usr/bin/gnuplot
reset
fontsize = 11
set terminal pdf enhanced font "Verdana,7"
#set term postscript enhanced eps fontsize
set output "case_study_result_black.pdf"
set style fill solid 1.00 border 0
set style histogram errorbars gap 2 lw 1
set style data histogram
set xtics rotate by -30
set grid ytics
set grid xtics
set xlabel "All Functionality types implemented in TroGuard"
set ylabel "Probability"
set yrange [0:6]
#set key below
set ytics add("0" 0, "10%%" 1, "20%%" 2, "30%%" 3, "40%%" 4, "50%%" 5, "60%%" 6)
#"70%%" 7, "80%%" 8, "90%%" 0.9, "100%%" 1 )

#set datafile separator ","
#plot 'case_study_results.dat' using 2:3:xtic(1) ti "Trojan Game" linecolor rgb "#FF0000"
plot 'case_study_results.dat' using 2:3:xtic(1) ti "Trojan Game" linecolor rgb "black"
