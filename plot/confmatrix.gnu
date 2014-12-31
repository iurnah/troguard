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

set output 'all_attr.pdf'
#set output 'file_attr.pdf'
#set output 'resource_attr.pdf'
#set output 'network_attr.pdf'
#set output 'interactivity_attr.pdf'
#set output 'all_attr_reorder.pdf'

set border linewidth 1
unset key
set cbrange [0:100]
#unset colorbox
set palette gray negative
#set palette gradient

#draw the vertical lines
set arrow 1 from graph 0.1,0 to graph 0.1,1 nohead front lt 0
set arrow 2 from graph 0.2,0 to graph 0.2,1 nohead front lt 0
set arrow 3 from graph 0.3,0 to graph 0.3,1 nohead front lt 0
set arrow 4 from graph 0.4,0 to graph 0.4,1 nohead front lt 0
set arrow 5 from graph 0.5,0 to graph 0.5,1 nohead front lt 0
set arrow 6 from graph 0.6,0 to graph 0.6,1 nohead front lt 0
set arrow 7 from graph 0.7,0 to graph 0.7,1 nohead front lt 0
set arrow 8 from graph 0.8,0 to graph 0.8,1 nohead front lt 0
set arrow 9 from graph 0.9,0 to graph 0.9,1 nohead front lt 0

#draw the horizontal lines
set arrow 10 from graph 0,0.1 to graph 1,0.1 nohead front lt 0
set arrow 11 from graph 0,0.2 to graph 1,0.2 nohead front lt 0
set arrow 12 from graph 0,0.3 to graph 1,0.3 nohead front lt 0
set arrow 13 from graph 0,0.4 to graph 1,0.4 nohead front lt 0
set arrow 14 from graph 0,0.5 to graph 1,0.5 nohead front lt 0
set arrow 15 from graph 0,0.6 to graph 1,0.6 nohead front lt 0
set arrow 16 from graph 0,0.7 to graph 1,0.7 nohead front lt 0
set arrow 17 from graph 0,0.8 to graph 1,0.8 nohead front lt 0
set arrow 18 from graph 0,0.9 to graph 1,0.9 nohead front lt 0

set xrange [-0.5:9.5]
set yrange [-0.5:9.5]
set xtics add("1" 1, "3" 3, "5" 5, "7" 7, "9" 9)
set ytics add("1" 1, "3" 3, "5" 5, "7" 7, "9" 9)
set xlabel "Label in the Data Set"
set ylabel "Cross Validation (Classified As)"

plot 'confmatrix_all.dat'  matrix w image
#plot 'confmatrix_all.dat'  matrix w image
#plot 'confmatrix_file.dat'  matrix w image
#plot 'confmatrix_resource.dat'  matrix w image
#plot 'confmatrix_network.dat'  matrix w image
#plot 'confmatrix_interactivity.dat'  matrix w image

