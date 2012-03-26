set datafile separator ","
set title "Cluster 2 tracking" font ",13"
set xlabel "Experiment"
set ylabel "Normalized dimension"
set xtics 1
set xrange [1:2]
set yrange [0:1]
plot '/gpfs/projects/bsc41/bsc41127/TrackClusters.2012/tests/MR_GENESIS.TRACKING.RESULTS.Cluster2.csv' using 1:2 w linespoints lw 3 title 'IPC',\
'/gpfs/projects/bsc41/bsc41127/TrackClusters.2012/tests/MR_GENESIS.TRACKING.RESULTS.Cluster2.csv' using 1:3 w linespoints lw 3 title 'PM_INST_CMPL'
pause -1 "Hit return to continue..."
