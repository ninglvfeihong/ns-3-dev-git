set pm3d map
set palette
set key off
set xlabel "time (ms)"
set ylabel "freq (MHz)"
set zlabel "PSD (dBW/Hz)" offset 15,0,0
set xrange [160 to 260]
splot "./spectrum-analyzer-output-2-0.tr" using ($1*1000.0):($2/1e6):(10*log10($3))
