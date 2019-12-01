set title "LR-WPAN packet delay vs Lr-Wpan slot length"
set xlabel "Lr-Wpan slot (ms)"
set ylabel "LR-WPAN Packet Delay (ms)"
set xrange [0:32]
set grid
set for [i=1:7] style line 1 lw 1 ps 1
set style increment user
plot "-"  title "Sender number:1" with linespoints, "-"  title "Sender number:2" with linespoints, "-"  title "Sender number:3" with linespoints, "-"  title "Sender number:4" with linespoints, "-"  title "Sender number:5" with linespoints, "-"  title "Sender number:6" with linespoints, "-"  title "Sender number:7" with linespoints
1 0
5 39.9379
9 18.3141
13 12.5377
17 10.05
21 8.42301
25 7.41717
29 6.67598
e
1 0
5 75.6227
9 36.2196
13 24.9388
17 19.827
21 16.4973
25 14.3252
29 12.9543
e
1 0
5 116.75
9 55.2091
13 37.6097
17 29.2766
21 24.2836
25 20.6611
29 18.3304
e
1 0
5 155.779
9 71.9521
13 49.5526
17 37.9391
21 30.9579
25 25.919
29 22.9328
e
1 0
5 190.085
9 88.0271
13 60.2473
17 46.1255
21 37.1062
25 30.6941
29 26.8636
e
1 0
5 217.304
9 100.831
13 70.561
17 52.4163
21 41.9862
25 34.7688
29 29.9272
e
1 0
5 232.45
9 113.345
13 78.1354
17 58.8249
21 46.2824
25 37.6223
29 32.8575
e
