set title "LR-WPAN Packet Count (packet/slot) vs Lr-Wpan slot length"
set xlabel "Lr-Wpan slot (ms)"
set ylabel "Packet Count (packet/slot)"
set xrange [0:32]
set grid
set key left
set for [i=1:7] style line 1 lw 1 ps 1
set style increment user
plot "-"  title "Sender number:1" with linespoints, "-"  title "Sender number:2" with linespoints, "-"  title "Sender number:3" with linespoints, "-"  title "Sender number:4" with linespoints, "-"  title "Sender number:5" with linespoints, "-"  title "Sender number:6" with linespoints, "-"  title "Sender number:7" with linespoints
1 0
5 1.77529
9 4.23415
13 6.71504
17 9.04464
21 11.5763
25 14.0427
29 16.5884
e
1 0
5 1.99855
9 4.64276
13 7.27495
17 9.75177
21 12.3503
25 15.0274
29 17.6111
e
1 0
5 2.02697
9 4.80626
13 7.53665
17 10.1548
21 12.8471
25 15.6163
29 18.2834
e
1 0
5 2.01349
9 4.9026
13 7.66029
17 10.3834
21 13.1289
25 15.9831
29 18.7132
e
1 0
5 1.9829
9 4.92828
13 7.72122
17 10.5061
21 13.2865
25 16.178
29 19.0199
e
1 0
5 1.98459
9 4.90072
13 7.70503
17 10.5276
21 13.3505
25 16.2731
29 19.1515
e
1 0
5 1.97278
9 4.84078
13 7.66706
17 10.5488
21 13.3891
25 16.2799
29 19.1038
e
