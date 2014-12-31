#!/usr/bin/gnuplot
#
# Plotting a color map
#
# AUTHOR: Hagen Wierstorf
# MODIFIED: Rui Han

reset

# png
#set terminal pngcairo size 500,500 enhanced font 'Verdana,12'
#set output 'all_attr.png'
#set output 'file_attr.png'
#set output 'resource_attr.png'
#set output 'network_attr.png'
#set output 'interactivity_attr.png'

#PDF
set terminal pdf size 6,5 enhanced font 'Verdana,10'
#set terminal pdf size 5,5 enhanced font 'Verdana,12'

set output 'confmatrix_coreutils.pdf'

set border linewidth 1
unset key
set cbrange [0:100]
#set cbrange [0:100]
#unset colorbox
set palette gray negative
#set palette gradient

#draw the vertical lines
set arrow 1 from graph 0.25,0 to graph 0.25,1 nohead front lt 0
set arrow 2 from graph 0.5,0 to graph 0.5,1 nohead front lt 0
set arrow 3 from graph 0.75,0 to graph 0.75,1 nohead front lt 0
set arrow 4 from graph 1,0 to graph 1,1 nohead front lt 0

#draw the horizontal lines
set arrow 10 from graph 0,0.25 to graph 1,0.25 nohead front lt 0
set arrow 11 from graph 0,0.5 to graph 1,0.5 nohead front lt 0
set arrow 12 from graph 0,0.75 to graph 1,0.75 nohead front lt 0
set arrow 13 from graph 0,1 to graph 1,1 nohead front lt 0


set xrange [-0.5:3.5]
set yrange [-0.5:3.5]
unset xtics
set xtics 1
set ytics 1
#set xtics add("0" 0, "1" 1, "2" 5, "3" 7)
#set ytics add("0" 1, "1" 3, "2" 5, "3" 7,)
set xlabel "Label in the Data Set"
set ylabel "Cross Validation (Classified As)"


plot 'confmatrix_coreutils.dat'  matrix w image
