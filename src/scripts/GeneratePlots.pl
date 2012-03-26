#!/usr/bin/perl

use Cwd 'abs_path';

require "ParaverColors.pl";

# Defines
my $SEQUENCE_FILE_SUFFIX     = ".SEQUENCES";
my $CLUSTER_DATA_FILE_SUFFIX = ".clustered.csv";
my $CLUSTER_INFO_FILE_SUFFIX = ".clusters_info.csv";
my $PARAVER_OFFSET           = 4;

my $SUM      = "SUM";
my $DENSITY  = "DENSITY";
my $MIN      = "MIN";
my $MAX      = "MAX";
my $AVG_NORM = "NORM";

# Parse arguments
my $ARGC             = @ARGV;
my $OUT_PREFIX       = $ARGV[$ARGC-1];
my $NUM_INPUT_TRACES = $ARGC-1;

my $SEQUENCE_FILE = $OUT_PREFIX.$SEQUENCE_FILE_SUFFIX;

open (FD, $SEQUENCE_FILE) || die "Could not open '$SEQUENCE_FILE': $!\n";

my %Sequences = ();

my $CurrentCluster = 1;

while ($CurrentSequence = <FD>)
{
  chomp $CurrentSequence;

  my @SequenceTokens = split(/\s+<->\s+/, $CurrentSequence);

  my $StepsInSequence = @SequenceTokens;
  if ($StepsInSequence != $NUM_INPUT_TRACES)
  {
    print "ERROR: Number of input traces (".$NUM_INPUT_TRACES.") differs from the number of experiments in the cluster sequence (".$StepsInSequence.")\n";
    exit;
  }
  for (my $Step = 0; $Step < $StepsInSequence; $Step++)
  {
    my $ClusterSet = $SequenceTokens[$Step];
    my @ClusterSetArray = split(/,/, $ClusterSet);
    foreach $Cluster (@ClusterSetArray)
    {
      push(@{$Sequences{$CurrentCluster}{$Step}}, $Cluster);
    }
    #print "Sequences{$CurrentCluster}{$Step} = $ClusterSet\n";
  }
  $CurrentCluster ++;
}
my $NUM_FINAL_CLUSTERS = $CurrentCluster-1;

my %Statistics = ();
my %UniqueDimensions = ();

for (my $CurrentTrace = 0; $CurrentTrace < $NUM_INPUT_TRACES; $CurrentTrace++) 
{
  my $TraceBaseName = $ARGV[$CurrentTrace];
  my $ClusterDataFile = $TraceBaseName.$CLUSTER_DATA_FILE_SUFFIX;

  open (DATA, $ClusterDataFile) || die "Could not open '$ClusterDataFile: $!\n'"; 

  my @Dimensions = ();
  my $NumberOfDimensions = 0;

  my $LineNo = 0;
  my $TotalBursts = 0;
  while ($Line = <DATA>)
  {
    chomp $Line;
    if ($LineNo == 0) 
    {
      # Parse the dimensions labels
      @Dimensions = split(/\s*,\s*/, $Line);
      $NumberOfDimensions = @Dimensions;
      #print "Dims=".$NumberOfDimensions."\n";
      for (my $i=0; $i<$NumberOfDimensions; $i++)
      {
        $UniqueDimensions{$Dimensions[$i]} = 1;
      }
    }
    else
    {
      # Parse the dimensions values
      $TotalBursts ++;

      my @DimensionsValues = split(/,/, $Line);
      my $ClusterID = $DimensionsValues[$DimensionsValues-1] - $PARAVER_OFFSET;
      for (my $i=0; $i<scalar(@DimensionsValues)-1; $i++)
      {
        my $CurrentDimension = $Dimensions[$i];
        my $CurrentValue     = $DimensionsValues[$i];

        #print $Dimensions[$i]." = ".$DimensionsValues[$i]."\n";
        
        if (! exists $Statistics{$CurrentDimension}{$CurrentTrace}{$ClusterID})
        {
          $Statistics{$CurrentDimension}{$CurrentTrace}{$ClusterID}{$SUM}     = $CurrentValue;
          $Statistics{$CurrentDimension}{$CurrentTrace}{$ClusterID}{$DENSITY} = 1;
          $Statistics{$CurrentDimension}{$CurrentTrace}{$ClusterID}{$MIN}     = $CurrentValue;
          $Statistics{$CurrentDimension}{$CurrentTrace}{$ClusterID}{$MAX}     = $CurrentValue;
        }
        else
        {
          $Statistics{$CurrentDimension}{$CurrentTrace}{$ClusterID}{$SUM} += $CurrentValue;
          $Statistics{$CurrentDimension}{$CurrentTrace}{$ClusterID}{$DENSITY} ++;
          if ($CurrentValue < $Statistics{$CurrentDimension}{$CurrentTrace}{$ClusterID}{$MIN})
          {
            $Statistics{$CurrentDimension}{$CurrentTrace}{$ClusterID}{$MIN} = $CurrentValue;
          }
          if ($CurrentValue > $Statistics{$CurrentDimension}{$CurrentTrace}{$ClusterID}{$MAX})
          {
            $Statistics{$CurrentDimension}{$CurrentTrace}{$ClusterID}{$MAX} = $CurrentValue;
          }
        } 
        # Compute the average so far
        $Statistics{$CurrentDimension}{$CurrentTrace}{$ClusterID}{$AVG} = 
          $Statistics{$CurrentDimension}{$CurrentTrace}{$ClusterID}{$SUM} / 
          $Statistics{$CurrentDimension}{$CurrentTrace}{$ClusterID}{$DENSITY};

        # Normalize the average so far
        if ($Statistics{$CurrentDimension}{$CurrentTrace}{$ClusterID}{$MIN} == $Statistics{$CurrentDimension}{$CurrentTrace}{$ClusterID}{$MAX})
        {
          $Statistics{$CurrentDimension}{$CurrentTrace}{$ClusterID}{$AVG_NORM} = 1;
        } 
        else 
        {
          $Statistics{$CurrentDimension}{$CurrentTrace}{$ClusterID}{$AVG_NORM} = 
            ($Statistics{$CurrentDimension}{$CurrentTrace}{$ClusterID}{$AVG} - $Statistics{$CurrentDimension}{$CurrentTrace}{$ClusterID}{$MIN}) /
            ($Statistics{$CurrentDimension}{$CurrentTrace}{$ClusterID}{$MAX} - $Statistics{$CurrentDimension}{$CurrentTrace}{$ClusterID}{$MIN});
        }
      }
    }
    $LineNo ++;
  }
  close DATA;
}

foreach $Dimension (sort keys %UniqueDimensions)
{
  print "Writing tendency lines for metric '$Dimension'\n";

  my $DimensionDataFile = $OUT_PREFIX.".".$Dimension.".csv";
  my $DimensionDispFile = $OUT_PREFIX.".".$Dimension.".dispersion.csv";
  my $DimensionPlotFile = $OUT_PREFIX.".".$Dimension.".gnuplot";

  open (DATA, ">$DimensionDataFile") || die "Could not create '$DimensionDataFile: $!\n'"; 
  open (DISP, ">$DimensionDispFile") || die "Could not create '$DimensionDispFile: $!\n'"; 
  
  foreach $FinalClusterID (sort { $a <=> $b } keys %Sequences)
  {
    #print "FINAL_CLUSTER $FinalClusterID\n";
    foreach $Step (sort { $a <=> $b } keys %{$Sequences{$FinalClusterID}})
    {
      #print "STEP $Step\n";

      @ClusterSet = @{$Sequences{$FinalClusterID}{$Step}};

      my $DimensionWeightedAvg = 0;
      my $TotalDensity         = 0;
      my $SetDimensionMin      = -1; 
      my $SetDimensionMax      = -1;
      foreach $ClusterID (@ClusterSet)
      {
        #print "CLUSTER $ClusterID\n";
        #print "DIMENSION $Dimension\n";
        #print "MIN=".$Statistics{$Dimension}{$Step}{$ClusterID}{$MIN}."\n";
        #print "MAX=".$Statistics{$Dimension}{$Step}{$ClusterID}{$MAX}."\n";
        #print "SUM=".$Statistics{$Dimension}{$Step}{$ClusterID}{$SUM}."\n";
        #print "DENSITY=".$Statistics{$Dimension}{$Step}{$ClusterID}{$DENSITY}."\n";
        #print "AVG=".$Statistics{$Dimension}{$Step}{$ClusterID}{$AVG}."\n";
        my $Min = $Statistics{$Dimension}{$Step}{$ClusterID}{$MIN};
        if (($Min < $SetDimensionMin) || ($SetDimensionMin == -1))
        {
          $SetDimensionMin = $Min;
        }
        my $Max = $Statistics{$Dimension}{$Step}{$ClusterID}{$MAX};
        if (($Max > $SetDimensionMax) || ($SetDimensionMax == -1))
        {
          $SetDimensionMax = $Max;
        }
        my $Density = $Statistics{$Dimension}{$Step}{$ClusterID}{$DENSITY};
        my $Avg     = $Statistics{$Dimension}{$Step}{$ClusterID}{$AVG};
        $DimensionWeightedAvg += $Avg * $Density;
        $TotalDensity += $Density;
      }
      if ($TotalDensity != 0)
      {
        $DimensionWeightedAvg = $DimensionWeightedAvg / $TotalDensity;
        my $DimensionDispersion = $SetDimensionMax - $SetDimensionMin;
        print DATA ($Step+1).",".$DimensionWeightedAvg.",".($FinalClusterID+$PARAVER_OFFSET)."\n";
        print DISP ($Step+1).",".$DimensionDispersion.",".($FinalClusterID+$PARAVER_OFFSET)."\n";
      }
    }
  }

  close DATA;
  close DISP;

  open (PLOT, ">$DimensionPlotFile") || die "Could not create '$DimensionPlotFile: $!\n'";
  print PLOT "set size 0.5, 1\n";
  print PLOT "set datafile separator \",\"\n";
  print PLOT "set multiplot\n";
  print PLOT "set origin 0, 0\n";
  print PLOT "set title \"$Dimension tracking\" font \",13\"\n";
  print PLOT "set xlabel \"Experiments\"\n";
  print PLOT "set ylabel \"Weighted $Dimension\"\n";
  print PLOT "set key below\n";
  print PLOT "set xtics 1\n";
  print PLOT "plot ";
  for (my $i=0; $i<$NUM_FINAL_CLUSTERS; $i++)
  {
    print PLOT "'".abs_path($DimensionDataFile)."' using 1:(\$3 == ".($i+1+$PARAVER_OFFSET)." ? \$2 : 1/0) w linespoints lw 3 lt rgbcolor \"".GetParaverColor($i+1+$PARAVER_OFFSET)."\" title 'Cluster ".($i+1)."'";
    if ($i < $NUM_FINAL_CLUSTERS-1)
    {
      print PLOT ",\\";
    }
    print PLOT "\n";
  }
  print PLOT "set size 0.5, 1\n";
  print PLOT "set origin 0.5, 0\n";
  print PLOT "set title \"$Dimension dispersion tracking\" font \",13\"\n";
  print PLOT "set xlabel \"Experiments\"\n";
  print PLOT "set ylabel \"$Dimension dispersion\"\n";
  print PLOT "set key below\n";
  print PLOT "set xtics 1\n";
  print PLOT "plot ";
  for (my $i=0; $i<$NUM_FINAL_CLUSTERS; $i++)
  {
    print PLOT "'".abs_path($DimensionDispFile)."' using 1:(\$3 == ".($i+1+$PARAVER_OFFSET)." ? \$2 : 1/0) w linespoints lw 3 lt rgbcolor \"".GetParaverColor($i+1+$PARAVER_OFFSET)."\" title 'Cluster ".($i+1)."'";
    if ($i < $NUM_FINAL_CLUSTERS-1)
    {
      print PLOT ",\\";
    }
    print PLOT "\n";
  }

  print PLOT "set nomultiplot\n";
  print PLOT "pause -1 \"Hit return to continue...\"\n";
  
  close PLOT;
}

#sub getStat()
#{
#    my ($Stat, $AllStats) = @_;
#
#    $Value = `echo "$AllStats" | grep "$Stat" | cut -d= -f 2`;
#    chomp $Value;
#    return $Value;
#}
#
#my $ARGC = @ARGV;
#my $NUM_SEQUENCE   = $ARGV[0];
#my $NUM_TRACE      = $ARGV[1];
#my $OUT_PREFIX     = $ARGV[2];
#my $TRACE_BASENAME = $ARGV[3];
#my $CLUSTERS_GROUP = join( " ", @ARGV[4,$ARGC-1] );
#
#my $Stats = `@TRACKCLUSTERS_PREFIX@/aux_scripts/ComputeStats.pl $TRACE_BASENAME $CLUSTERS_GROUP`;
#
#$IPC           = &getStat("Weighted_IPC", $Stats);
#$IPCDispersion = &getStat("Delta_IPC",    $Stats);
#
#open  OUT_IPC, ">>$OUT_PREFIX.IPC.data";
#print OUT_IPC ($NUM_TRACE+1).",".$IPC.",".($NUM_SEQUENCE+1)."\n";
#close OUT_IPC;
#
#open  OUT_IPC_DISP, ">>$OUT_PREFIX.IPC.Dispersion.data";
#print OUT_IPC_DISP ($NUM_TRACE+1).",".$IPCDispersion.",".($NUM_SEQUENCE+1)."\n";
#close OUT_IPC_DISP;
#

my %ExtrapolationStatistics = ();

for (my $CurrentTrace = 0; $CurrentTrace < $NUM_INPUT_TRACES; $CurrentTrace++) 
{
  my $TraceBaseName = $ARGV[$CurrentTrace];

  my $ClusterInfoFile = $TraceBaseName.$CLUSTER_INFO_FILE_SUFFIX;
  open (INFO, $ClusterInfoFile) || die "Could not open '$ClusterInfoFile: $!\n'"; 

  my $LineNo = 0;
  my $FirstClusterIndex = 0;
  my @DensityArray = ();
  while ($Line = <INFO>)
  {
    chomp $Line;

    my @LineTokens = split(/,/, $Line);
    if ($LineTokens[0] eq "Cluster Name")
    {
      my @ClusterNamesArray = @LineTokens;
      for (my $i=0; $i<@ClusterNamesArray; $i++)
      {
        if ("$ClusterNamesArray[$i]" eq "Cluster 1")
        {
          $FirstClusterIndex = $i;
          last
        }
      }
    }
    elsif ($LineTokens[0] eq "Density")
    {
      @DensityArray = @LineTokens;
    }
    elsif ($LineNo > 4)
    {
      my @ExtrapolationValuesArray = @LineTokens;
      my $ExtrapolationDimension = $ExtrapolationValuesArray[0];
      my $MinAvgAllClusters = -1;
      my $MaxAvgAllClusters = -1;
      for (my $ClusterID=$FistClusterIndex; $ClusterID<@ExtrapolationValuesArray; $ClusterID++)
      {
        if (($ExtrapolationValuesArray[$ClusterID] < $MinAvgAllClusters) || ($MinAvgAllClusters == -1))
        {
          $MinAvgAllClusters = $ExtrapolationValuesArray[$ClusterID];
        }
        if (($ExtrapolationValuesArray[$ClusterID] > $MaxAvgAllClusters) || ($MaxAvgAllClusters == -1))
        {
          $MaxAvgAllClusters = $ExtrapolationValuesArray[$ClusterID];
        }
      }
      for (my $ClusterID=$FistClusterIndex; $ClusterID<@ExtrapolationValuesArray; $ClusterID++)
      {
        $ExtrapolationStatistics{$ExtrapolationDimension}{$CurrentTrace}{$ClusterID}{$AVG}      = $ExtrapolationValuesArray[$ClusterID];
        $ExtrapolationStatistics{$ExtrapolationDimension}{$CurrentTrace}{$ClusterID}{$DENSITY}  = $DensityArray[$ClusterID];
        $ExtrapolationStatistics{$ExtrapolationDimension}{$CurrentTrace}{$ClusterID}{$AVG_NORM} = 
          ($ExtrapolationValuesArray[$ClusterID] - $MinAvgAllClusters) / ($MaxAvgAllClusters - $MinAvgAllClusters);
      }
    }
  }
  close INFO;
}

foreach $FinalClusterID (sort { $a <=> $b } keys %Sequences)
{
  my $ClusterTrackingDataFile = $OUT_PREFIX.".Cluster".$FinalClusterID.".csv";
  my $ClusterTrackingPlotFile = $OUT_PREFIX.".Cluster".$FinalClusterID.".gnuplot";

  open (DATA, ">$ClusterTrackingDataFile") || die "Could not create '$ClusterTrackingDataFile: $!\n'";
  open (PLOT, ">$ClusterTrackingPlotFile") || die "Could not create '$ClusterTrackingPlotFile: $!\n'";

  print DATA "Experiment";
  foreach $ClusteringDimension (sort keys %Statistics)
  {
    print DATA ",$ClusteringDimension";
  }
  foreach $ExtrapolationDimension (sort keys %ExtrapolationStats)
  {
    print DATA ",$ExtrapolationDimension";
  }
  print DATA "\n";

  print PLOT "set datafile separator \",\"\n".
             "set title \"Cluster $FinalClusterID tracking\" font \",13\"\n".
             "set xlabel \"Experiment\"\n".
             "set ylabel \"Normalized dimension\"\n".
             "set xtics 1\n".
             "set xrange [1:$NUM_INPUT_TRACES]\n".
             "set yrange [0:1]\n".
             "plot ";

  for (my $Step=0; $Step<$NUM_INPUT_TRACES; $Step++)
  {
    print DATA ($Step+1);
    foreach $ClusteringDimension (sort keys %Statistics)
    {
      my @ClusterSet = @{$Sequences{$FinalClusterID}{$Step}};
      my $SetWeightedAverage = 0;
      my $SetTotalDensity = 0;
      foreach $ClusterID (@ClusterSet)
      {
        $SetWeightedAverage += $Statistics{$ClusteringDimension}{$Step}{$ClusterID}{$AVG_NORM} * $Statistics{$ClusteringDimension}{$Step}{$ClusterID}{$DENSITY};
        $SetTotalDensity += $Statistics{$ClusteringDimension}{$Step}{$ClusterID}{$DENSITY};
      }
      if ($SetTotalDensity == 0) 
      {
        $SetWeightedAverage = 0;
      }
      else
      {
        $SetWeightedAverage = $SetWeightedAverage / $SetTotalDensity;
      }
      print DATA ",".$SetWeightedAverage;
    }
    foreach $ExtrapolationDimension (sort keys %ExtrapolationStatistics)
    {
      my @ClusterSet = @{$Sequences{$FinalClusterID}{$Step}};
      my $SetWeightedAverage = 0;
      my $SetTotalDensity = 0;
      foreach $ClusterID (@ClusterSet)
      {
        $SetWeightedAverage += $ExtrapolationStatistics{$ExtrapolationDimension}{$Step}{$ClusterID}{$AVG_NORM} * $ExtrapolationStatistics{$ExtrapolationDimension}{$Step}{$ClusterID}{$DENSITY};
        $SetTotalDensity += $ExtrapolationStatistics{$ExtrapolationDimension}{$Step}{$ClusterID}{$DENSITY};
      }
      $SetWeightedAverage = $SetWeightedAverage / $SetTotalDensity;
      print DATA ",".$SetWeightedAverage
    }
    print DATA "\n";
  }

  my $CurrentPlottedDimension = 1;
  my $PLOT_DATA="";
  foreach $ClusteringDimension (sort keys %Statistics)
  {
    $PLOT_DATA .= "'".abs_path($ClusterTrackingDataFile)."' using 1:".($CurrentPlottedDimension+1)." w linespoints lw 3 title '$ClusteringDimension',\\\n";
    $CurrentPlottedDimension ++;
  }
  foreach $ExtrapolationDimension (sort keys %ExtrapolationStats)
  {
    $PLOT_DATA .= "'".abs_path($ClusterTrackingDataFile)."' using 1:".($CurrentPlottedDimension+1)." w linespoints lw 3 title '$ExtrapolationDimension',\\\n";
    $CurrentPlottedDimension ++;
  }
  chop $PLOT_DATA; chop $PLOT_DATA; chop $PLOT_DATA;
  print PLOT $PLOT_DATA."\n";
  print PLOT "pause -1 \"Hit return to continue...\"\n";

  close DATA;
  close PLOT;
}

