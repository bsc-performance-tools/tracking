#!/usr/bin/perl

# Defines
my $SEQUENCE_FILE_SUFFIX       = ".SEQUENCES";
my $RECOLORED_PLOT_SUFFIX      = ".recolored.plot";
my $RECOLORED_DATA_SUFFIX      = ".recolored.data";
my $RECOLORED_MULTIPLOT_SUFFIX = ".recolored.multiplot";
my $TO_PNG_SUFFIX              = ".topng";
my $PARAVER_OFFSET             = 5;
my $CLUSTER_NOISE              = 0;
my $CLUSTER_THRESHOLD_FILTERED = -1;

my $SECTION_INFO          = "[Info]";
my $SECTION_TRACKED       = "[Tracked]";
my $SECTION_FILTERED      = "[Filtered]";
my $SECTION_UNTRACKED     = "[Untracked]";
my $VAR_FRAMES            = "Frames";
my $VAR_OBJECTS           = "Objects";

# Parse arguments
my $ARGC             = @ARGV;
my $OUT_PREFIX       = $ARGV[$ARGC-1];
my $NUM_INPUT_PLOTS  = $ARGC-1;

my %ColorTranslation   = ();
my @RecoloredPlotArray = ();

# Read the cluster sequence file to build the color translation table 
my $SEQUENCE_FILE = $OUT_PREFIX.$SEQUENCE_FILE_SUFFIX;
open (FD, $SEQUENCE_FILE) || die "Could not open '$SEQUENCE_FILE': $!\n";
my $CurrentCluster = 1;

my $NumFrames  = 0;
my $NumObjects = 0;

do
{
  $Line = <FD>;
  chomp $Line;

  if ((substr($Line, 0, 1) eq "[") && (substr($Line, -1, 1) eq "]"))
  {
    $CurrentSection = $Line;
  }
  elsif ((length($Line) > 0) && (index($Line, '=') != -1))
  {
    my @Tokens = split('=', $Line);
    my $Var    = $Tokens[0];
    my $Value  = $Tokens[1];
    
    if ($Var eq $VAR_FRAMES)
    {
      $NumFrames = $Value;
    }
    elsif ($Var eq $VAR_OBJECTS)
    {
      $NumObjects = $Value;
      if ($CurrentSection eq $SECTION_TRACKED)
      {
        $NUM_CLUSTERS = $NumObjects;
      }
    }
    else 
    {
      my $Object = $Var;
      my $Links  = $Value;

      my @LinksTokens = split(/;/, $Links);
      my $FramesInLinks = @LinksTokens;
      if ($FramesInLinks != $NUM_INPUT_PLOTS)
      {
        print "ERROR: Number of input plots (".$NUM_INPUT_PLOTS.") differs from the number of experiments in the cluster sequence (".$FramesInLinks.")\n";
        exit;
      }

      if ($CurrentSection eq $SECTION_TRACKED)
      {
        $CurrentCluster = $Object;
      }
      elsif ($CurrentSection eq $SECTION_FILTERED)
      {
        $CurrentCluster = $CLUSTER_THRESHOLD_FILTERED;
      }
      elsif ($CurrentSection eq $SECTION_UNTRACKED)
      {
        $CurrentCluster = $CLUSTER_NOISE;
      }

      for (my $CurrentFrame = 0; $CurrentFrame < $FramesInLinks; $CurrentFrame++)
      {
        my $ClusterSet = $LinksTokens[$CurrentFrame];
   
        @ClusterSetArray = split(/,/, $ClusterSet);
        for (my $i = 0; $i < @ClusterSetArray; $i++)
        {
          $ColorTranslation{$CurrentFrame}{$ClusterSetArray[$i]} = $CurrentCluster;
        }
        $ColorTranslation{$CurrentFrame}{$CLUSTER_NOISE} = $CLUSTER_NOISE;
      }
    }
  }
} until eof;

close FD;

print "Recoloring clusters in ".$NUM_INPUT_PLOTS." plots...\n";

for (my $Step = 0; $Step < $NUM_INPUT_PLOTS; $Step++) {
  my $IN_PLOT = $ARGV[$Step];
  $IN_PLOT    = `ls $IN_PLOT`;
  chomp $IN_PLOT;
  my $DataFullPath = `dirname $IN_PLOT`;
  chomp $DataFullPath;

  print "Generating GNUplot script from template '$IN_PLOT'\n";
  open IN,  "$IN_PLOT" || die "Could not open '$IN_PLOT': $!\n";
  my $OUT_PLOT = $IN_PLOT.$RECOLORED_PLOT_SUFFIX;
  my $TO_PNG_SCRIPT = $OUT_PLOT.$TO_PNG_SUFFIX;
  open OUT, ">$OUT_PLOT" || die "Could not open '$OUT_PLOT': $!\n";
  open TOPNG, ">$TO_PNG_SCRIPT" || die "Could not open '$TO_PNG_SCRIPT': $!\n";

  push(@RecoloredPlotArray, $OUT_PLOT);

  my $ORIGINAL_DATA_FILE="";
  my $RECOLORED_DATA_FILE="";

  while (my $Line = <IN>)
  {
    if (substr($Line, 0, 4) eq "plot")
    {
      @LineTokens = split(/\s+/, $Line);
      $LineTokens[1] =~ s/^.(.*).$/$1/; # Remove first and last character '
      $ORIGINAL_DATA_FILE  = $LineTokens[1];
      $ORIGINAL_DATA_FILE  = $DataFullPath."/".$ORIGINAL_DATA_FILE;
      $RECOLORED_DATA_FILE = $ORIGINAL_DATA_FILE.$RECOLORED_DATA_SUFFIX;

      print OUT "plot ";

      print TOPNG "set notitle\n";
      print TOPNG "set nokey\n";
      print TOPNG "set nogrid\n";
      print TOPNG "set noxlabel\n";
      print TOPNG "set noylabel\n";
      print TOPNG "set noxtics\n";
      print TOPNG "set noytics\n";
      print TOPNG "set terminal png size 100,100 enhanced font \"Helvetica,20\"\n";
      print TOPNG "set output 'frame_".($Step+1).".png'\n";
      print TOPNG "plot ";
		
      if (grep /title \"Noise\"/, $Line)
      {
        $Line = <IN>; # Skip noise 
      }
      else
      {
        $Line = substr($Line, 5);
      }
      $CountClusters = 0;

#      if (grep /title \"Noise\"/, $Line)
#      {
#        $CountClusters --;
#      }
#      $Line = substr($Line, 5);

      do
      {
        @tmp = split(" ", $Line);
        splice(@tmp, 0, 1);
        $PlotArgs = join(" ", @tmp);
    
        if (($CountClusters == $NUM_CLUSTERS-1) && (substr($PlotArgs, -2) eq ",\\"))
        {
          chop $PlotArgs; chop $PlotArgs;
        }
        print OUT "'$RECOLORED_DATA_FILE' $PlotArgs\n";
        print TOPNG "'$RECOLORED_DATA_FILE' $PlotArgs\n";
  
        $Line = <IN>;
        $CountClusters ++;
      } while ($CountClusters < $NUM_CLUSTERS);
      print OUT "pause -1 \"Hit return to continue...\"\n";

      last; # Breaks the loop
    }
    else
    {
      print OUT $Line;
      print TOPNG $Line;
    }
  }
  close (IN);
  close (OUT);
  close (TOPNG);

  open IN,  "$ORIGINAL_DATA_FILE" || die "Could not open '$ORIGINAL_DATA_FILE': $!\n";
  open OUT, ">$RECOLORED_DATA_FILE" || die "Could not open '$RECOLORED_DATA_FILE': $!\n";

  my $CurrentLine = 0;
  while (my $Line = <IN>)
  {
    chomp $Line;
    if ($CurrentLine == 0)
    {
        print OUT $Line."\n";
    }
    else
    {
      my @LineTokens = split(/,/, $Line);
      my $Cluster = $LineTokens[@LineTokens-1];
      $Cluster -= $PARAVER_OFFSET;

      $NewColor = 0;
      if (exists $ColorTranslation{$Step}{$Cluster})
      {
        $NewColor = $ColorTranslation{$Step}{$Cluster} + $PARAVER_OFFSET;
      }
      else 
      {
        $NewColor = $NOISE + $PARAVER_OFFSET;
      }
      $LineTokens[@LineTokens-1] = $NewColor;
      $Line = join(",", @LineTokens);
      print OUT $Line."\n";
    }
    $CurrentLine ++;
  }
  close (IN);
  close (OUT);
}

print "Generating multiplot...\n";

my $PlotsPerRow = int(sqrt($NUM_INPUT_PLOTS)+0.99);

my $SizeX = 1/$PlotsPerRow;
my $SizeY = 1/int(($NUM_INPUT_PLOTS / $PlotsPerRow)+0.99);

my $RECOLORED_MULTIPLOT = $OUT_PREFIX.$RECOLORED_MULTIPLOT_SUFFIX;

open  $MULTI, ">$RECOLORED_MULTIPLOT";
print $MULTI "set multiplot\n";
print $MULTI "set nokey\n";
close $MULTI;

my $X=0;
my $Y=0;
for (my $i=0; $i<$NUM_INPUT_PLOTS; $i++)
{
  open  $MULTI, ">>$RECOLORED_MULTIPLOT";
  print $MULTI "set size $SizeX,$SizeY\n";
  print $MULTI "set origin $X,".(1-$SizeY-$Y)."\n";
  print $MULTI "set logscale y\n";
  close $MULTI;

  `cat $RecoloredPlotArray[$i] | grep -v pause | grep -v xlabel | grep -v ylabel | grep -v "set title" | grep -v key >> $RECOLORED_MULTIPLOT`;

  if (($i+1) % $PlotsPerRow == 0)
  {
    $X = 0;
    $Y += $SizeY;
  }
  else
  {
    $X += $SizeX;
  }
}

open  $MULTI, ">>$RECOLORED_MULTIPLOT";
print $MULTI "set nomultiplot\n";
print $MULTI "pause -1 \"Press any key to exit...\"\n";
close $MULTI;

print "Recoloring done!\n";

exit;
