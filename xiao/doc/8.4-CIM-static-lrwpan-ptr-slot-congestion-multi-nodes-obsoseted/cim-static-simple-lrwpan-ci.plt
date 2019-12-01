set title "LR-WPAN congestion indicator vs Lr-Wpan slot length"
set xlabel "Lr-Wpan slot (ms)"
set ylabel "Congestion indicator"
set xrange [0:32]
set grid
set key left
set for [i=1:7] style line 1 lw 1 ps 1
set style increment user
plot "-"  title "Sender number:1" with linespoints, "-"  title "Sender number:2" with linespoints, "-"  title "Sender number:3" with linespoints, "-"  title "Sender number:4" with linespoints, "-"  title "Sender number:5" with linespoints, "-"  title "Sender number:6" with linespoints, "-"  title "Sender number:7" with linespoints
1 0
5 0.614006
9 0.752017
13 0.807647
17 0.823713
21 0.848129
25 0.860809
29 0.874101
e
1 0
5 0.681165
9 0.820301
13 0.872424
17 0.886273
21 0.903565
25 0.920045
29 0.927139
e
1 0
5 0.689713
9 0.847624
13 0.902701
17 0.921929
21 0.939144
25 0.955478
29 0.962005
e
1 0
5 0.685656
9 0.863722
13 0.917005
17 0.942157
21 0.959326
25 0.977545
29 0.984299
e
1 0
5 0.676456
9 0.868015
13 0.924055
17 0.95301
21 0.970612
25 0.989266
29 1.0002
e
1 0
5 0.676963
9 0.863409
13 0.922182
17 0.954913
21 0.975197
25 0.994991
29 1.00703
e
1 0
5 0.673413
9 0.853392
13 0.917788
17 0.956788
21 0.977962
25 0.995397
29 1.00455
e
