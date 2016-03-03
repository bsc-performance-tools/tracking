#!/usr/bin/perl

my $ARGC = @ARGV;

my $SuffixClustersData   = ".DATA.csv";
my $SuffixClustersInfo   = ".clusters_info.csv";
my $SuffixClusteredTrace = ".clustered.prv";
my $SuffixAlign          = ".seq";
my $SuffixPCF            = ".pcf";
my $SuffixPRV            = ".prv";

my $CallersTemplate    = $ENV{'TRACKING_HOME'}."/etc/@sub_CALLERS_TEMPLATE@";
my $CALLERS_EVENT      = 80000000;

my $CallersLevel        = 0;
my $CallersCFG          = "";
my $ClusteringConfig    = "";
my $ListInput           = "";
my $DimensionsToScale   = "";
my $MinTimePct          = 0;
my $OutputPrefix        = "";
my $Reconstruct         = 1;
my $ScoreMinimum        = "";
my $Verbose             = 0;
my $MaxDistance         = 1.00;
my $Threshold           = "";

my @InputTraces           = ();
my $NumberOfTraces        = 0;
my @LastClustersToProcess = ();

### Prints help
sub PrintUsage
{
  (my $name = `basename "$0"`) =~ s/\.[^.]+$//;
  chomp $name;

  print "\nSYNTAX\n";
  print "  ".$name." [OPTIONS] [-l LIST | TRACE1 TRACE2 ...]\n";
  print "\nOPTIONS\n";
  print "  -a MIN_SCORE\n";
  print "        Minimum SPMD score to use the alignment tracker\n";
  print "  -c CALLER_LEVEL\n";
  print "        Enable the callstack tracker at the specified depth.\n";
  print "  -d    Enable the density tracker.\n";
  print "  -m MIN_TIME_PCT\n";
  print "        Discard the clusters below the given duration time percentage.\n";
  print "  -o OUTPUT_PREFIX\n";
  print "        Set the prefix for all the output files.\n";
  print "  -p MAX_DISTANCE\n";
  print "        Maximum Epsilon distance to use the position tracker\n";
#  print "  -r    Enable the trace reconstruction with tracked clusters.\n";
  print "  -s DIM1,DIM2...\n";
  print "        Select the dimensions to scale with the number of tasks (DEPRECATED).\n";
  print "  -t THRESHOLD\n";
  print "        Minimum likeliness percentage in order to match two clusters (special values: all | first).\n";
  print "  -v[v] Run in verbose mode (-vv for extra debug messages).\n";
  print "  -x CLUSTERING_CONFIG_XML\n";
  print "        Specify the clustering configuration to automatically cluster the traces.\n";
  print "\n";
}

### Reads the clusters info file to see the percentage of time that takes every cluster,
### to find which is the last cluster to trace if we are filtering below the given 
### time percentage.
sub FilterClustersByTime
{
    my ($ClustersInfo, $MinPctTime) = @_;

    my $PctDuration = `cat $ClustersInfo | grep "^%"`;
    chomp $PctDuration;
    my @arrayPctDuration = split (/,/, $PctDuration);

    my $ClusterName = `cat $ClustersInfo | grep "^Cluster Name"`;
    chomp $ClusterName;
    my @arrayClusterName = split (/,/, $ClusterName);

    my $i = $#arrayClusterName;
    my $found = 0;
    while (($i > 0) && ($found == 0))
    {
        if (($arrayPctDuration[$i] * 100) > $MinPctTime)
        {
            $found = 1;
        }
        else {
            $i --;
        }
    }

    if (!$found)
    {
        print "*** Error: All clusters are discarded! Try decreasing the minimum cluster time.\n";
        exit;
    }
    my @tmp = split (/ /, $arrayClusterName[$i]);
    my $LastClusterToTrack = $tmp[-1];

    return $LastClusterToTrack;
}

#########################################
###               MAIN                ###
#########################################

#
### Parse the input arguments
#
if ($ARGC < 2)
{
  PrintUsage();
  exit;
}

my $i = 0;
for ($i = 0; ($i < $ARGC) && (substr($ARGV[$i], 0, 1) eq '-'); $i++)
{
  $flag = substr($ARGV[$i], 1, 1);
  if ($flag == "a")
  {
    $i ++;
    $ScoreMinimum = $ARGV[$i];
  }
  elsif ($flag == "c")
  {
    $i ++;
    $CallersLevel = $ARGV[$i];
    if (($CallersLevel == 0 && $CallersLevel ne '0') || ($CallersLevel <= 0))
    {
      print "*** Error: -c: Callers level has to be greater than zero\n";
      exit;
    } 
  }
  elsif ($flag == "d")
  {
    $UseDensity = 1
  }
  elsif ($flag == "l") 
  {
    $i ++;
    $ListInput = $ARGV[$i];
  }
  elsif ($flag == "m")
  {
    $i ++;
    $MinTimePct = $ARGV[$i];
    if (($MinTimePct == 0 && $MinTimePct ne '0') || ($MinTimePct < 0) || ($MinTimePct > 100))
    {
      print "*** Error: -m: Minimum cluster time percentage has to be between 0 and 100\n";
      exit;
    }
  }
  elsif ($flag == "o")
  {
    $i ++;
    $OutputPrefix = $ARGV[$i];
  }
  elsif ($flag == "p")
  {
    $i ++;
    $MaxDistance = $ARGV[$i];
    if (($MaxDistance == 0 && $MaxDistance ne '0') || ($MaxDistance < 0) || ($MaxDistance > 1))
    {
      print "*** Error: -p: Distance has to be between 0 and 1\n";
      exit;
    }
  }
  #elsif ($flag == "r")
  #{
  #  $Reconstruct = 1;
  #}
  elsif ($flag == "s")
  {
    $i ++;
    $DimensionsToScale = $ARGV[$i];
  }
  elsif ($flag == "t")
  {
    $i ++;
    $Threshold = $ARGV[$i];
    $Threshold = lc $Threshold;
    if ((($Threshold == 0 && $Threshold ne '0') || ($Threshold < 0) || ($Threshold > 100)) && ($Threshold ne 'any') && ($Threshold ne 'first'))
    {
      print "*** Error: -t: Threshold has to be a percentage between 0 and 100, 'any' or 'first'\n";
      exit;
    }
  }
  elsif ($flag == "v")
  {
    $Verbose = 1;
    if (substr($ARGV[$i], 2, 1) eq 'v')
    {
      $Verbose ++;
    }
  }
  elsif ($flag == "x")
  {
    $i ++;
    $ClusteringConfig = $ARGV[$i];
    if (! -e $ClusteringConfig)
    {
      print "*** Error: -x: Can't find clustering definition XML file '$ClusteringConfig'\n";
      exit;
    }
  }
  else
  {
    PrintUsage();
    print "*** INVALID PARAMETER ".$ARGV[$i]."\n";
    exit;
  }
}	

#
### Parse the input traces and ensure there's at least two traces to track
#
if ($ListInput ne "") 
{
  open(LIST, $ListInput) or die("*** Error: Could not open file '$ListInput': $!\n");
  @InputTraces = <LIST>;
  close LIST;
  if ($InputTraces < 2)
  {
    PrintUsage();
    print "*** Error: Pass two traces or more to track!\n";
    exit;
  }
}
elsif ($ARGC - $i < 2)
{
  PrintUsage();
  print "*** Error: Pass two traces or more to track!\n";
  exit;
}
else
{
  while ($i < $ARGC)
  {
    push (@InputTraces, $ARGV[$i]);
    $i ++;
  }
}

#
### Check all the traces exist
#
$NumberOfTraces = @InputTraces;
for ($i=0; $i<$NumberOfTraces; $i++)
{
  if (! -e $InputTraces[$i])
  {
    print "*** Error: Trace '$InputTraces[$i]' not found.\n\n";
    exit;
  }
}

#
### Check if the traces are clustered
#
print "+ Checking input traces... ";

my $ClusteredTraces  = 0;

for ($i=0; $i<$NumberOfTraces; $i++)
{
  my $CurrentTrace   = $InputTraces[$i];
  (my $TraceBasename = $CurrentTrace) =~ s/\.[^.]+$//;

  # Check the CSV data and the clusters info files exist
  if ((-e $TraceBasename.$SuffixClustersData) and (-e $TraceBasename.$SuffixClustersInfo))
  {
    $ClusteredTraces ++;
  }
}

if ($ClusteredTraces == 0)
{
  # Any trace is clustered, cluster them automatically using the refinement.
  print "not clustered!\n";
  if ($ClusteringConfig eq "")
  {
    PrintUsage();
    print "*** Error: The traces specified are not clustered. Please specify a clustering definition file with -d CLUSTERING_CONFIG_XML to cluster them automatically, or pass clustered traces.\n";
    exit;
  }
  # Cluster each trace 
  for ($i=0; $i<$NumberOfTraces; $i++)
  {
    my $CurrentTrace   = $InputTraces[$i];
    (my $TraceBasename = $CurrentTrace) =~ s/\.[^.]+$//;

    print "+ Clustering trace ".($i+1)."...\n";
    #print $ENV{'CLUSTERING_HOME'}."/bin/BurstClustering -d $ClusteringConfig -a -ra -s -i $CurrentTrace -o $TraceBasename$SuffixClusteredTrace\n\n";
    `$ENV{'CLUSTERING_HOME'}/bin/BurstClustering -d $ClusteringConfig -a -ra -s -i $CurrentTrace -o $TraceBasename$SuffixClusteredTrace`;
    $InputTraces[$i] = $TraceBasename.$SuffixClusteredTrace;
  }
}
elsif ($ClusteredTraces != $NumberOfTraces)
{
  print "ERROR!\n";
  print "*** Error: Some of the traces you passed are clustered, but not all! Pass them either way, but don't mix!\n";
  exit;
}
else
{
  # All traces are clustered

  # Check the same dimensions were used in all clusterings
  my $ReferenceDimensions = "";
  for ($i=0; $i<$NumberOfTraces; $i++)
  {
    my $CurrentTrace   = $InputTraces[$i];
    (my $TraceBasename = $CurrentTrace) =~ s/\.[^.]+$//;
    my $CSVFile = $TraceBasename.$SuffixClustersData;

    my $ClusteringDimensions = `head -n1 $CSVFile`;
    if ($i == 0)
    {
      $ReferenceDimensions = $ClusteringDimensions;
    }
    else
    {
      if ($ReferenceDimensions ne $ClusteringDimensions)
      { 
        print "ERROR!\n";
        print "*** Error: Trace ".($i+1)." was clustered with different dimensions! ";
        print "Please cluster all traces with the same dimensions.\n\n";
        print "Expected:\n";
        print "   $ReferenceDimensions";
        print "Actual dimensions:\n";
        print "   $ClusteringDimensions";
        exit;
      }
    }
  }

  # All traces are clustered ok
  print "OK!\n";

  if ($ClusteringConfig ne "")
  {
    print "*** Warning: Ignoring parameter -d, all traces are clustered already!\n";
  }
}

#
### Print configuration
#
print "\n+ Tracking configuration:\n";
print "... Sequence of ".$NumberOfTraces." traces\n";
print "... Minimum cluster time: ".$MinTimePct."%\n";
print "... Reconstruct traces: ".($Reconstruct == 1 ? "enabled" : "disabled")."\n";
print "... Position tracking: ".($MaxDistance > 0 ? "cross-classifying with radius $MaxDistance" : "disabled")."\n";
print "... Callers tracking: ".($CallersLevel > 0 ? "enabled for level $CallersLevel" : "disabled")."\n";
print "... Alignment tracking: ".($ScoreMinimum ne "" ? "enabled above score $ScoreMinimum" : "disabled")."\n";
print "... Density tracking: ".($UseDensity == 1 ? "enabled" : "disabled")."\n";
print "... Verbose: ".($Verbose == 1 ? "yes" : "no")."\n";
print "\n";

#
### Scale/normalize plots and CSVs 
#
print "+ Scaling clustering frames... ";

my $CMD = "python ".$ENV{'TRACKING_HOME'}."/bin/@sub_SCRIPT_SCALE_FRAMES@ ";
if ($DimensionsToScale ne "") 
{
  $CMD .= "-s $DimensionsToScale ";
}
$CMD .= join(" ", @InputTraces);
system($CMD);
if ($? == 0) 
{
  print "OK!\n";
}
else 
{
  print "ERROR!\n";
  print "Command was: $CMD\n";
  exit;
}

#
### Check the alignment has been computed for all traces
#
print "\n+ Checking for alignments... ";

my $AllAligned = 1;

for ($i=0; $i<@InputTraces; $i++)
{
  my $CurrentTrace = $InputTraces[$i];
  (my $TraceWithoutExtension = $CurrentTrace) =~ s/\.[^.]+$//;
  my $AlignFile    = $TraceWithoutExtension.$SuffixAlign;

  if (! -e $AlignFile)
  {
    print "NO!\n";
    print "*** Warning: Can't find alignment file '$AlignFile'. Alignment tracking will be disabled...\n";
    $AllAligned = 0;
    last;
  }
}
print "YES!\n";

#
### Filter clusters below a minimum time percentage
#
print "\n+ Filtering clusters under $MinTimePct% of time...\n";
for ($i=0; $i<@InputTraces; $i++)
{
  my $CurrentTrace           = $InputTraces[$i];
  (my $TraceWithoutExtension = $CurrentTrace) =~ s/\.[^.]+$//;
  my $ClustersInfo = $TraceWithoutExtension.$SuffixClustersInfo;

  my $LastTraceCluster = FilterClustersByTime($ClustersInfo, $MinTimePct);
  push (@LastClustersToProcess, $LastTraceCluster);
  my $str = `basename $CurrentTrace`;
  chomp $str;
  print "$LastTraceCluster clusters will be tracked for trace ".($i+1)." ($str)\n";
}

#
### Check if the traces have callers
#
if ($CallersLevel > 0)
{
  my $CallersEventType = $CALLERS_EVENT + $CallersLevel;	

  print "\n+ Checking whether traces have level $CallersLevel callstack events... ";
  for ($i=0; $i<@InputTraces; $i++)
  {
    my $CurrentTrace           = $InputTraces[$i];
    (my $TraceWithoutExtension = $CurrentTrace) =~ s/\.[^.]+$//;
    my $TracePCF = $TraceWithoutExtension.$SuffixPCF;
    my $TracePRV = $TraceWithoutExtension.$SuffixPRV;

    # Check first the PCF
    my $CallersPresentPCF = `cat $TracePCF | grep -m 1 \"$CallersEventType\"`;
    if ($CallersPresentPCF eq "")
    {
      $CallersLevel = 0;
      last;
    }
    else
    {
      # If they appear in the PCF, check the PRV
      my $CallersPresentPRV = `cat $TracePRV | grep -m 1 \":$CallersEventType:\"`;
      if ($CallersPresentPRV eq "")
      {
        $CallersLevel = 0;
        last;
      }
    }
  }

  if ($CallersLevel > 0)
  {
    print "YES!\n";

    print "+ Generating callers CFG... ";
   
    $CallersCFG = "callers-lvl-$CallersLevel.cfg";
    `cat $CallersTemplate | sed "s/\@sub_CALLERS_LEVEL\@/$CallersEventType/g" > $CallersCFG`;
    print "$CallersCFG\n";
  }
  else
  {
    print "NO!\n";
    print "*** Warning: Callstack tracking will be disabled\n";
  }
}

#
### Invoking the tracking algorithm
#
print "\n+ Tracking clusters...\n";

$CMD=$ENV{'TRACKING_HOME'}."/bin/tracking.bin ";

if ($ScoreMinimum ne "")
{
  $CMD .= "-a $ScoreMinimum ";
}
if ($CallersLevel > 0)
{
  $CMD .= "-c $CallersCFG ";
}
if ($UseDensity == 1)
{
  $CMD .= "-d ";
}
if ($MaxDistance ne "")
{
  $CMD .= "-p $MaxDistance ";
}
if ($MinTimePct > 0)
{
  $CMD .= "-m $MinTimePct ";
}
if ($Reconstruct == 1)
{
  $CMD .= "-r ";
}
if ($Threshold ne "")
{
  if ($Threshold eq "any") 
  {
    $Threshold = 0;
  }
  elsif ($Threshold eq "first")
  {
    $Threshold = -1;
  }
  $CMD .= "-t $Threshold ";
}
if ($Verbose == 1)
{
  $CMD .= "-v ";
}
if ($Verbose == 2)
{
  $CMD .= "-vv ";
}
if ($OutputPrefix ne "")
{
  $CMD .= "-o $OutputPrefix ";
}

for ($i=0; $i<@InputTraces; $i++)
{
  # This appends the last cluster to process per trace according to the percentage of time they represent
  $CMD .= $InputTraces[$i].":".$LastClustersToProcess[$i]." ";
}

system($CMD);
 
