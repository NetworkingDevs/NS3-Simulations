set terminal pngcairo enhanced font 'Verdana,12'
set output 'bar_plot_queuing_delay.png'

# Replace with your titles and settings...
set title 'Enqueue Time'
set ylabel 'Enququq (Time)'
set xlabel 'Packet Number (Index)'
set style fill solid

# replace with your file...
plot 'Test.txt' using 1:2 with boxes
