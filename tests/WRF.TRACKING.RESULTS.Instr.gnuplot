set size 0.5, 1
set datafile separator ","
set multiplot
set origin 0, 0
set title "Instr tracking" font ",13"
set xlabel "Experiments"
set ylabel "Weighted Instr"
set key below
set xtics 1
plot '/gpfs/projects/bsc41/bsc41127/TrackClusters.2012/tests/WRF.TRACKING.RESULTS.Instr.csv' using 1:($3 == 5 ? $2 : 1/0) w linespoints lw 3 lt rgbcolor "#00FF00" title 'Cluster 1',\
'/gpfs/projects/bsc41/bsc41127/TrackClusters.2012/tests/WRF.TRACKING.RESULTS.Instr.csv' using 1:($3 == 6 ? $2 : 1/0) w linespoints lw 3 lt rgbcolor "#FFFF00" title 'Cluster 2',\
'/gpfs/projects/bsc41/bsc41127/TrackClusters.2012/tests/WRF.TRACKING.RESULTS.Instr.csv' using 1:($3 == 7 ? $2 : 1/0) w linespoints lw 3 lt rgbcolor "#EB0000" title 'Cluster 3',\
'/gpfs/projects/bsc41/bsc41127/TrackClusters.2012/tests/WRF.TRACKING.RESULTS.Instr.csv' using 1:($3 == 8 ? $2 : 1/0) w linespoints lw 3 lt rgbcolor "#00A200" title 'Cluster 4',\
'/gpfs/projects/bsc41/bsc41127/TrackClusters.2012/tests/WRF.TRACKING.RESULTS.Instr.csv' using 1:($3 == 9 ? $2 : 1/0) w linespoints lw 3 lt rgbcolor "#FF00FF" title 'Cluster 5',\
'/gpfs/projects/bsc41/bsc41127/TrackClusters.2012/tests/WRF.TRACKING.RESULTS.Instr.csv' using 1:($3 == 10 ? $2 : 1/0) w linespoints lw 3 lt rgbcolor "#6464B1" title 'Cluster 6',\
'/gpfs/projects/bsc41/bsc41127/TrackClusters.2012/tests/WRF.TRACKING.RESULTS.Instr.csv' using 1:($3 == 11 ? $2 : 1/0) w linespoints lw 3 lt rgbcolor "#ACAE29" title 'Cluster 7',\
'/gpfs/projects/bsc41/bsc41127/TrackClusters.2012/tests/WRF.TRACKING.RESULTS.Instr.csv' using 1:($3 == 12 ? $2 : 1/0) w linespoints lw 3 lt rgbcolor "#FF901A" title 'Cluster 8',\
'/gpfs/projects/bsc41/bsc41127/TrackClusters.2012/tests/WRF.TRACKING.RESULTS.Instr.csv' using 1:($3 == 13 ? $2 : 1/0) w linespoints lw 3 lt rgbcolor "#02FFB1" title 'Cluster 9',\
'/gpfs/projects/bsc41/bsc41127/TrackClusters.2012/tests/WRF.TRACKING.RESULTS.Instr.csv' using 1:($3 == 14 ? $2 : 1/0) w linespoints lw 3 lt rgbcolor "#C0E000" title 'Cluster 10'
set size 0.5, 1
set origin 0.5, 0
set title "Instr dispersion tracking" font ",13"
set xlabel "Experiments"
set ylabel "Instr dispersion"
set key below
set xtics 1
plot '/gpfs/projects/bsc41/bsc41127/TrackClusters.2012/tests/WRF.TRACKING.RESULTS.Instr.dispersion.csv' using 1:($3 == 5 ? $2 : 1/0) w linespoints lw 3 lt rgbcolor "#00FF00" title 'Cluster 1',\
'/gpfs/projects/bsc41/bsc41127/TrackClusters.2012/tests/WRF.TRACKING.RESULTS.Instr.dispersion.csv' using 1:($3 == 6 ? $2 : 1/0) w linespoints lw 3 lt rgbcolor "#FFFF00" title 'Cluster 2',\
'/gpfs/projects/bsc41/bsc41127/TrackClusters.2012/tests/WRF.TRACKING.RESULTS.Instr.dispersion.csv' using 1:($3 == 7 ? $2 : 1/0) w linespoints lw 3 lt rgbcolor "#EB0000" title 'Cluster 3',\
'/gpfs/projects/bsc41/bsc41127/TrackClusters.2012/tests/WRF.TRACKING.RESULTS.Instr.dispersion.csv' using 1:($3 == 8 ? $2 : 1/0) w linespoints lw 3 lt rgbcolor "#00A200" title 'Cluster 4',\
'/gpfs/projects/bsc41/bsc41127/TrackClusters.2012/tests/WRF.TRACKING.RESULTS.Instr.dispersion.csv' using 1:($3 == 9 ? $2 : 1/0) w linespoints lw 3 lt rgbcolor "#FF00FF" title 'Cluster 5',\
'/gpfs/projects/bsc41/bsc41127/TrackClusters.2012/tests/WRF.TRACKING.RESULTS.Instr.dispersion.csv' using 1:($3 == 10 ? $2 : 1/0) w linespoints lw 3 lt rgbcolor "#6464B1" title 'Cluster 6',\
'/gpfs/projects/bsc41/bsc41127/TrackClusters.2012/tests/WRF.TRACKING.RESULTS.Instr.dispersion.csv' using 1:($3 == 11 ? $2 : 1/0) w linespoints lw 3 lt rgbcolor "#ACAE29" title 'Cluster 7',\
'/gpfs/projects/bsc41/bsc41127/TrackClusters.2012/tests/WRF.TRACKING.RESULTS.Instr.dispersion.csv' using 1:($3 == 12 ? $2 : 1/0) w linespoints lw 3 lt rgbcolor "#FF901A" title 'Cluster 8',\
'/gpfs/projects/bsc41/bsc41127/TrackClusters.2012/tests/WRF.TRACKING.RESULTS.Instr.dispersion.csv' using 1:($3 == 13 ? $2 : 1/0) w linespoints lw 3 lt rgbcolor "#02FFB1" title 'Cluster 9',\
'/gpfs/projects/bsc41/bsc41127/TrackClusters.2012/tests/WRF.TRACKING.RESULTS.Instr.dispersion.csv' using 1:($3 == 14 ? $2 : 1/0) w linespoints lw 3 lt rgbcolor "#C0E000" title 'Cluster 10'
set nomultiplot
pause -1 "Hit return to continue..."
