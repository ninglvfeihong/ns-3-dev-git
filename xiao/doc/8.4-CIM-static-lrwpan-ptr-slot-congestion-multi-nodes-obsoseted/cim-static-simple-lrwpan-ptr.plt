set title "LR-WPAN Packet Transmission Rate (PTR) vs Lr-Wpan slot length"
set xlabel "Lr-Wpan slot (ms)"
set ylabel "Packet Transmission Rate (packet/s)"
set xrange [0:32]
set grid
set key left
set for [i=1:7] style line 1 lw 1 ps 1
set style increment user
plot "-"  title "Sender number:1" with linespoints, "-"  title "Sender number:2" with linespoints, "-"  title "Sender number:3" with linespoints, "-"  title "Sender number:4" with linespoints, "-"  title "Sender number:5" with linespoints, "-"  title "Sender number:6" with linespoints, "-"  title "Sender number:7" with linespoints
1 0
5 24.7034
9 53.0297
13 76.4495
17 94.3931
21 111.522
25 125.612
29 138.515
e
1 0
5 26.0004
9 55.4025
13 79.7004
17 98.2103
21 115.226
25 130.233
29 142.429
e
1 0
5 24.1504
9 53.8508
13 79.1306
17 98.2908
21 115.812
25 131.443
29 144.172
e
1 0
5 21.238
9 51.6321
13 76.6506
17 97.4462
21 115.138
25 130.954
29 143.803
e
1 0
5 18.3189
9 49.1655
13 74.6062
17 95.5761
21 113.061
25 129.483
29 143.193
e
1 0
5 16.0366
9 47.044
13 71.4894
17 93.83
21 111.137
25 127.341
29 141.146
e
1 0
5 14.1397
9 44.2456
13 70.0114
17 91.4371
21 109.418
25 125.146
29 138.243
e
