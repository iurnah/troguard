#!/usr/bin/gnuplot
# Plotting a color map
#
# AUTHOR: Hagen Wierstorf

reset
# png
set terminal pngcairo size 500,500 enhanced font 'Verdana,12'
set output 'grid.png'
set grid 
set grid 
set border linewidth 0
unset key
#set cbrange [0:60]
unset colorbox
set palette gray negative
#set palette gradient

#set xrange [-0.5:9.5]
#set yrange [-0.5:9.5]

set xrange [0:10]
set yrange [0:10]
#unset xtics
#unset ytics
set xtics add("1" 1, "3" 3, "5" 5, "7" 7, "9" 9) textcolor rgb "white"
set ytics add("1" 1, "3" 3, "5" 5, "7" 7, "9" 9) textcolor rgb "white"
set xlabel "Lable in the Data Set" textcolor rgb "white"
set ylabel "Cross Validation (Classified As)" textcolor rgb "white"
plot 'void.dat' matrix linecolor rgb "white"
