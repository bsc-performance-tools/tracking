set size 0.5, 1
set datafile separator ","
set multiplot
set origin 0, 0
set title "PM_INST_CMPL tracking" font ",13"
set xlabel "Experiments"
set ylabel "Weighted PM_INST_CMPL"
set key below
set xtics 1
plot '/gpfs/projects/bsc41/bsc41127/TrackClusters.2012/tests/MR_GENESIS.TRACKING.RESULTS.PM_INST_CMPL.csv' using 1:($3 == 5 ? $2 : 1/0) w linespoints lw 3 lt rgbcolor "#00FF00" title 'Cluster 1',\
'/gpfs/projects/bsc41/bsc41127/TrackClusters.2012/tests/MR_GENESIS.TRACKING.RESULTS.PM_INST_CMPL.csv' using 1:($3 == 6 ? $2 : 1/0) w linespoints lw 3 lt rgbcolor "#FFFF00" title 'Cluster 2',\
'/gpfs/projects/bsc41/bsc41127/TrackClusters.2012/tests/MR_GENESIS.TRACKING.RESULTS.PM_INST_CMPL.csv' using 1:($3 == 7 ? $2 : 1/0) w linespoints lw 3 lt rgbcolor "#EB0000" title 'Cluster 3'
set size 0.5, 1
set origin 0.5, 0
set title "PM_INST_CMPL dispersion tracking" font ",13"
set xlabel "Experiments"
set ylabel "PM_INST_CMPL dispersion"
set key below
set xtics 1
plot '/gpfs/projects/bsc41/bsc41127/TrackClusters.2012/tests/MR_GENESIS.TRACKING.RESULTS.PM_INST_CMPL.dispersion.csv' using 1:($3 == 5 ? $2 : 1/0) w linespoints lw 3 lt rgbcolor "#00FF00" title 'Cluster 1',\
'/gpfs/projects/bsc41/bsc41127/TrackClusters.2012/tests/MR_GENESIS.TRACKING.RESULTS.PM_INST_CMPL.dispersion.csv' using 1:($3 == 6 ? $2 : 1/0) w linespoints lw 3 lt rgbcolor "#FFFF00" title 'Cluster 2',\
'/gpfs/projects/bsc41/bsc41127/TrackClusters.2012/tests/MR_GENESIS.TRACKING.RESULTS.PM_INST_CMPL.dispersion.csv' using 1:($3 == 7 ? $2 : 1/0) w linespoints lw 3 lt rgbcolor "#EB0000" title 'Cluster 3'
set nomultiplot
pause -1 "Hit return to continue..."
