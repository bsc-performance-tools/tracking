#!/usr/bin/perl

use Cwd 'abs_path';

require "ParaverColors.pl";

# Defines
my $SEQUENCE_FILE_SUFFIX     = ".SEQUENCES";
my $CLUSTER_DATA_FILE_SUFFIX = ".clustered.csv";
my $CLUSTER_INFO_FILE_SUFFIX = ".clusters_info.csv";
my $PARAVER_OFFSET           = 4;

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
my %ClusteringDimensions = ();

# Clustering dimensions
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
        $ClusteringDimensions{$Dimensions[$i]} = 1;
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
        
        if (! exists $Statistics{$CurrentDimension}{$ClusterID}{$CurrentTrace})
        {
          $Statistics{$CurrentDimension}{$ClusterID}{$CurrentTrace}{SUM}       = 0;
          $Statistics{$CurrentDimension}{$ClusterID}{$CurrentTrace}{DENSITY}   = 0;
          $Statistics{$CurrentDimension}{$ClusterID}{$CurrentTrace}{LOCAL_MIN} = $CurrentValue;
          $Statistics{$CurrentDimension}{$ClusterID}{$CurrentTrace}{LOCAL_MAX} = $CurrentValue;
        }
        $Statistics{$CurrentDimension}{$ClusterID}{$CurrentTrace}{SUM}     += $CurrentValue;
        $Statistics{$CurrentDimension}{$ClusterID}{$CurrentTrace}{DENSITY} ++;
        if ($CurrentValue < $Statistics{$CurrentDimension}{$ClusterID}{$CurrentTrace}{LOCAL_MIN})
        {
          $Statistics{$CurrentDimension}{$ClusterID}{$CurrentTrace}{LOCAL_MIN} = $CurrentValue;
        }
        if ($CurrentValue > $Statistics{$CurrentDimension}{$ClusterID}{$CurrentTrace}{LOCAL_MAX})
        {
          $Statistics{$CurrentDimension}{$ClusterID}{$CurrentTrace}{LOCAL_MAX} = $CurrentValue;
        }
        
        # Update the average so far
        my $CurrentAvg = $Statistics{$CurrentDimension}{$ClusterID}{$CurrentTrace}{SUM} /
                         $Statistics{$CurrentDimension}{$ClusterID}{$CurrentTrace}{DENSITY};
        $Statistics{$CurrentDimension}{$ClusterID}{$CurrentTrace}{AVG} = $CurrentAvg;
      }
    }
    $LineNo ++;
  }

  close DATA;
}

# Extrapolated metrics
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

      my $C = $FirstClusterIndex;
      my $Tokens = @ExtrapolationValuesArray;
      for ($idx=$FirstClusterIndex; $idx<@ExtrapolationValuesArray; $idx++)
      {
        my $ClusterID = $idx - $FirstClusterIndex + 1;
        $Statistics{$ExtrapolationDimension}{$ClusterID}{$CurrentTrace}{AVG}     = $ExtrapolationValuesArray[$idx];
        $Statistics{$ExtrapolationDimension}{$ClusterID}{$CurrentTrace}{DENSITY} = $DensityArray[$idx];
      }
    }
    $LineNo ++;
  }
  close INFO;
}


#foreach $Dimension (sort keys %Statistics)
#{
#  for (my $ClusterID=1; $ClusterID<=$NUM_FINAL_CLUSTERS; $ClusterID++)
#  {
#    for (my $Trace=0; $Trace<$NUM_INPUT_TRACES; $Trace++)
#    {
#print "DIM=$Dimension CID=$ClusterID Trace=$Trace AVG=".$Statistics{$Dimension}{$ClusterID}{$Trace}{AVG}." DENSITY=".$Statistics{$Dimension}{$ClusterID}{$Trace}{DENSITY}."\n";
#    }
#  }
#}
#exit;
#
## Normalize the averages of the clustering dimensions and the extrapolated metrics with respect to 
## the min/max averages for that cluster across all experiments
#
#foreach $Dimension (sort keys %Statistics)
#{
##  foreach $ClusterID (sort keys %{$Statistics{$Dimension}})
#  for (my $ClusterID=1; $ClusterID<=$NUM_FINAL_CLUSTERS; $ClusterID++)
#  {
#    for (my $Trace=0; $Trace<$NUM_INPUT_TRACES; $Trace++)
#    {
#      my $CurrentAvg = $Statistics{$Dimension}{$ClusterID}{$Trace}{AVG};
#
#      # Update the minimum and maximum average per cluster across all experiments
#      if ((! exists $Statistics{$Dimension}{$ClusterID}{CLUSTER_AVG_MIN}) ||
#          ($CurrentAvg < $Statistics{$Dimension}{$ClusterID}{CLUSTER_AVG_MIN}))
#      {
#        $Statistics{$Dimension}{$ClusterID}{CLUSTER_AVG_MIN} = $CurrentAvg;
#      }
#      if ((! exists $Statistics{$Dimension}{$ClusterID}{CLUSTER_AVG_MAX}) ||
#          ($CurrentAvg > $Statistics{$Dimension}{$ClusterID}{CLUSTER_AVG_MAX}))
#      {
#        $Statistics{$Dimension}{$ClusterID}{CLUSTER_AVG_MAX} = $CurrentAvg;
#      }
#    }
#
#    # Normalize the averages
##    foreach $Trace (sort keys %{$Statistics{$Dimension}{$ClusterID}})
##    {
##      if ($Statistics{$Dimension}{$ClusterID}{CLUSTER_AVG_MIN} == $Statistics{$Dimension}{$ClusterID}{CLUSTER_AVG_MAX})
##      {
##        $Statistics{$Dimension}{$ClusterID}{$Trace}{AVG_NORM} = 0;
##      }
##      else
##      {
##        $Statistics{$Dimension}{$ClusterID}{$Trace}{AVG_NORM} =
##          ($Statistics{$Dimension}{$ClusterID}{$Trace}{AVG} - $Statistics{$Dimension}{$ClusterID}{CLUSTER_AVG_MIN}) /
##          ($Statistics{$Dimension}{$ClusterID}{CLUSTER_AVG_MAX} - $Statistics{$Dimension}{$ClusterID}{CLUSTER_AVG_MIN});
##      }
##    }
#    for (my $Trace=0; $Trace<$NUM_INPUT_TRACES; $Trace++)
#    {
#      $Statistics{$Dimension}{$ClusterID}{$Trace}{AVG_NORM} =
#        ($Statistics{$Dimension}{$ClusterID}{$Trace}{AVG} / $Statistics{$Dimension}{$ClusterID}{CLUSTER_AVG_MAX});
#
#print "DIM=$Dimension CID=$ClusterID Trace=$Trace AVG=".$Statistics{$Dimension}{$ClusterID}{$Trace}{AVG}." AVG_MAX=".$Statistics{$Dimension}{$ClusterID}{CLUSTER_AVG_MAX}." NORM=".$Statistics{$Dimension}{$ClusterID}{$Trace}{AVG_NORM}."\n";
#
##      $Statistics{$Dimension}{$ClusterID}{$Trace}{AVG_NORM} =
##        ($Statistics{$Dimension}{$ClusterID}{$Trace}{AVG} / $Statistics{$Dimension}{$ClusterID}{CLUSTER_AVG_MAX}) /
##        $Statistics{"PM_CYC"}{$ClusterID}{$Trace}{AVG};
#    }
#  }
#}




# Draw plots for clustering dimensions
foreach $Dimension (sort keys %ClusteringDimensions)
{
  print "Writing trend lines for metric '$Dimension'\n";

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
        my $Min = $Statistics{$Dimension}{$ClusterID}{$Step}{LOCAL_MIN};
        if (($Min < $SetDimensionMin) || ($SetDimensionMin == -1))
        {
          $SetDimensionMin = $Min;
        }
        my $Max = $Statistics{$Dimension}{$ClusterID}{$Step}{LOCAL_MAX};
        if (($Max > $SetDimensionMax) || ($SetDimensionMax == -1))
        {
          $SetDimensionMax = $Max;
        }
        my $Density = $Statistics{$Dimension}{$ClusterID}{$Step}{DENSITY};
        my $Avg     = $Statistics{$Dimension}{$ClusterID}{$Step}{AVG};
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



my %Normalized = ();
foreach $Dimension (sort keys %Statistics)
{
  for (my $FinalClusterID=1; $FinalClusterID<=$NUM_FINAL_CLUSTERS; $FinalClusterID++)
  {
    for (my $Step=0; $Step<$NUM_INPUT_TRACES; $Step++)
    {
      my @ClusterSet = @{$Sequences{$FinalClusterID}{$Step}};

      foreach $ClusterID (@ClusterSet)
      {
        my $CurrentAvg = $Statistics{$Dimension}{$ClusterID}{$Step}{AVG};

        if ((! exists $Normalized{$Dimension}{$FinalClusterID}{AVG_MIN}) ||
            ($CurrentAvg < $Normalized{$Dimension}{$FinalClusterID}{AVG_MIN}))
        {
          $Normalized{$Dimension}{$FinalClusterID}{AVG_MIN} = $CurrentAvg;
        }
        if ((! exists $Normalized{$Dimension}{$FinalClusterID}{AVG_MAX}) ||
            ($CurrentAvg > $Normalized{$Dimension}{$FinalClusterID}{AVG_MAX}))
        {
          $Normalized{$Dimension}{$FinalClusterID}{AVG_MAX} = $CurrentAvg;
        }
      }
    }
  }
}


# Draw plots per cluster
foreach $FinalClusterID (sort { $a <=> $b } keys %Sequences)
{
  my $ClusterTrackingDataFile    = $OUT_PREFIX.".Cluster".$FinalClusterID.".csv";
  my $ClusterTrackingRawDataFile = $OUT_PREFIX.".Cluster".$FinalClusterID.".raw.csv";
  my $ClusterTrackingPlotFile    = $OUT_PREFIX.".Cluster".$FinalClusterID.".gnuplot";

  open (DATA, ">$ClusterTrackingDataFile")    || die "Could not create '$ClusterTrackingDataFile: $!\n'";
  open (RAW,  ">$ClusterTrackingRawDataFile") || die "Could not create '$ClusterTrackingRawDataFile: $!\n'";
  open (PLOT, ">$ClusterTrackingPlotFile")    || die "Could not create '$ClusterTrackingPlotFile: $!\n'";

  print DATA "Experiment";
  print RAW  "Experiment";
  foreach $Dimension (sort keys %Statistics)
  {
    print DATA ",$Dimension";
    print RAW  ",$Dimension";
  }
  print DATA "\n";
  print RAW  "\n";

  print PLOT "set datafile separator \",\"\n".
             "set title \"Cluster $FinalClusterID tracking\" font \",13\"\n".
             "set xlabel \"Experiment\"\n".
             "set ylabel \"Normalized dimension\"\n".
             "set xtics 1\n".
             "set xrange [1:$NUM_INPUT_TRACES]\n".
             "plot ";

  for (my $Step=0; $Step<$NUM_INPUT_TRACES; $Step++)
  {
    print DATA ($Step+1);
    print RAW  ($Step+1);
    foreach $Dimension (sort keys %Statistics)
    {
      my @ClusterSet = @{$Sequences{$FinalClusterID}{$Step}};
      my $SetWeightedAverage = 0;
      my $SetWeightedRawAverage = 0;
      my $SetTotalDensity = 0;

      foreach $ClusterID (@ClusterSet)
      {
        my $NormalizedAverage   = $Statistics{$Dimension}{$ClusterID}{$Step}{AVG} / $Normalized{$Dimension}{$FinalClusterID}{AVG_MAX};
        $SetWeightedAverage    += $NormalizedAverage * $Statistics{$Dimension}{$ClusterID}{$Step}{DENSITY};
        $SetWeightedRawAverage += $Statistics{$Dimension}{$ClusterID}{$Step}{AVG} * $Statistics{$Dimension}{$ClusterID}{$Step}{DENSITY};
        $SetTotalDensity += $Statistics{$Dimension}{$ClusterID}{$Step}{DENSITY};
      }
      if ($SetTotalDensity == 0) 
      {
        $SetWeightedAverage    = 0;
        $SetWeightedRawAverage = 0;
      }
      else
      {
        $SetWeightedAverage    = $SetWeightedAverage / $SetTotalDensity;
        $SetWeightedRawAverage = $SetWeightedRawAverage / $SetTotalDensity;
      }
      print DATA ",".$SetWeightedAverage;
      print RAW  ",".$SetWeightedRawAverage;
    }
    print DATA "\n";
    print RAW  "\n";
  }

  my $CurrentPlottedDimension = 1;
  my $PLOT_DATA="";
  foreach $Dimension (sort keys %Statistics)
  {
    $PLOT_DATA .= "'".abs_path($ClusterTrackingDataFile)."' using 1:".($CurrentPlottedDimension+1)." w linespoints lw 3 title '$Dimension',\\\n";
    $CurrentPlottedDimension ++;
  }
  chop $PLOT_DATA; chop $PLOT_DATA; chop $PLOT_DATA;
  print PLOT $PLOT_DATA."\n";
  print PLOT "pause -1 \"Hit return to continue...\"\n";

  close DATA;
  close RAW;
  close PLOT;
}

