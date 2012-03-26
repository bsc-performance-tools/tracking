#!/usr/bin/perl

use Switch;

# Defines
my $FIXED_SCALE      = 0;
my $SCALE_WITH_TASKS = 1;
my $CSVSUFFIX        = ".clustered.csv";
my $PLOTSUFFIX       = ".*.gnuplot";

my $ARGC       = @ARGV;
my $currentArg = 0;

my @Metrics               = ();
my @MetricsFixScale       = ();

my @LocalMinDimensions    = ();		# @{ minDim1, minDim2 ... minDimN } for a given trace
my @LocalMaxDimensions    = ();		# @{ maxDim1, maxDim2 ... maxDimN } for a given trace
my %GlobalMinDimensions   = ();		# key: dimension	value: minimum for all traces
my %GlobalMaxDimensions   = ();		# key: dimension    value: maximum for all traces
my %MinDimensionsPerTrace = ();		# key: dimension	value: @{ minTrace1, minTrace2 ... minTraceN }
my %MaxDimensionsPerTrace = ();		# key: dimension    value: @{ maxTrace1, maxTrace2 ... maxTraceN }

sub PrintUsage
{
    print "Syntax: $0 [-f <metric_list>] <trace1.prv> <trace2.prv> ... <traceN.prv>\n";
}

if ($ARGC < 1) {
	PrintUsage();
    exit;
}

# Parse arguments
for ($currentArg = 0; ($currentArg < $ARGC) && (substr($ARGV[$currentArg], 0, 1) eq '-'); $currentArg++)
{
    switch (substr($ARGV[$currentArg], 1, 1))
    {
        case "f"
        {
            $currentArg ++;
			@MetricsFixScale = split(",", $ARGV[$currentArg]);
        }
        else
        {
            print "*** INVALID PARAMETER ".$ARGV[$currentArg]."\n";
            PrintUsage();
            exit;
        }
    }
}

if ($ARGC - $currentArg < 1)
{
    # 1 trace at least
    PrintUsage();
    exit;
}

# Parse the input traces
while ($currentArg < $ARGC)
{
    if (! -e $ARGV[$currentArg])
    {
        print "*** Error: Trace '$ARGV[$currentArg]' not found.\n";
        exit;
    }
    push (@InputTraces, $ARGV[$currentArg]);
    $currentArg ++;
}

for (my $i=0; $i<@InputTraces; $i++)
{
    my $Tracefile = $InputTraces[$i];
    (my $withoutExt = $Tracefile) =~ s/\.[^.]+$//;
    my $CSVFile = $withoutExt.$CSVSUFFIX;

    # Open data file
    open CSV, "$CSVFile" or die "$0: Error: Unable to open CSV file '$CSVFile'\n$!\n";
    my $currentLine = 0;

	@Metrics            = ();
	@LocalMinDimensions = ();
	@LocalMaxDimensions = ();

	# Compute every min and max dimension for this trace
    while (defined ($line = <CSV>))
    {
        chomp $line;
        if ($currentLine == 0)
        {
			$line =~ s/\s+//;
			@Metrics = split(/,/, $line);
		}
		else
		{
			@MetricsValues = split (/,/, $line);

			for (my $j=0; $j<@MetricsValues; $j++)
			{
				if ((! exists $LocalMinDimensions[$j]) || ($MetricsValues[$j] < $LocalMinDimensions[$j])) 
				{
					$LocalMinDimensions[$j] = $MetricsValues[$j];
				}
				elsif ((! exists $LocalMaxDimensions[$j]) || ($MetricsValues[$j] > $LocalMaxDimensions[$j]))
				{
					$LocalMaxDimensions[$j] = $MetricsValues[$j];
				}
			}
		}
		$currentLine ++;
	}
	# DEBUG
	for (my $j=0; $j<@Metrics; $j++)
	{
	    print "[Trace $i] Metric=$Metrics[$j] Min=$LocalMinDimensions[$j] Max=$LocalMaxDimensions[$j]\n";
	}

	close CSV;

	# Update global min and max for each dimension 
	for (my $j=0; $j<@Metrics; $j++)
	{
		my $currentMetric = $Metrics[$j];

		$GlobalMinDimensions{$currentMetric}   = $LocalMinDimensions[$j] if ! exists $GlobalMinDimensions{$currentMetric};
		$GlobalMaxDimensions{$currentMetric}   = $LocalMaxDimensions[$j] if ! exists $GlobalMaxDimensions{$currentMetric};
		$MinDimensionsPerTrace{$currentMetric} = () if ! exists $MinDimensionsPerTrace{$currentMetric};
		$MaxDimensionsPerTrace{$currentMetric} = () if ! exists $MaxDimensionsPerTrace{$currentMetric};

		$GlobalMinDimensions{$currentMetric} = $LocalMinDimensions[$j] if ($LocalMinDimensions[$j] < $GlobalMinDimensions{$currentMetric});
		$GlobalMaxDimensions{$currentMetric} = $LocalMaxDimensions[$j] if ($LocalMaxDimensions[$j] > $GlobalMaxDimensions{$currentMetric});
		push( @{$MinDimensionsPerTrace{$currentMetric}}, $LocalMinDimensions[$j] ); 
		push( @{$MaxDimensionsPerTrace{$currentMetric}}, $LocalMaxDimensions[$j] ); 
	}

	# Get the number of tasks from the trace
    $numTasks = `head -n1 $Tracefile | cut -d\\( -f3 | cut -d: -f3`;
    chomp $numTasks;
    push(@arrayNumTasks, $numTasks);
}

# DEBUG
for (my $i=0; $i<@Metrics; $i++)
{
	print "[GLOBAL] Metric=$Metrics[$i] Min=$GlobalMinDimensions{$Metrics[$i]} Max=$GlobalMaxDimensions{$Metrics[$i]}\n";
}

my %WeightedGlobalMinDimensions = ();	# key: dimension	value: global minimum weighted by the number of tasks 
my %WeightedGlobalMaxDimensions = ();	# key: dimension    value: global maximum weighted by the number of tasks 
my %TraceWithMinDimension       = ();	# key: dimension	value: number of trace that has the global minimum 
my %TraceWithMaxDimension       = ();	# key: dimension	value: number of trace that has the global maximum

# Compute the weighted dimensions per number of task 
for (my $i=0; $i<@Metrics; $i++)
{
	my $currentMetric = $Metrics[$i];

    for (my $currentTrace=0; $currentTrace<@InputTraces; $currentTrace++)
    {
		my $currentScaledMin = @{$MinDimensionsPerTrace{$currentMetric}}[$currentTrace] * $arrayNumTasks[$currentTrace];
		my $currentScaledMax = @{$MaxDimensionsPerTrace{$currentMetric}}[$currentTrace] * $arrayNumTasks[$currentTrace];

		if ((! exists $WeightedGlobalMinDimensions{$currentMetric}) || ($currentScaledMin < $WeightedGlobalMinDimensions{$currentMetric}))
		{
			$WeightedGlobalMinDimensions{$currentMetric} = $currentScaledMin;
			$TraceWithMinDimension{$currentMetric}       = $currentTrace;
		}
        if ((! exists $WeightedGlobalMaxDimensions{$currentMetric}) || ($currentScaledMax > $WeightedGlobalMaxDimensions{$currentMetric}))
        {
            $WeightedGlobalMaxDimensions{$currentMetric} = $currentScaledMax;
			$TraceWithMaxDimension{$currentMetric}       = $currentTrace;
		}
    }
}

my %MinRatios = ();		# key: dimension	value: @{ MinRatioTrace1, MinRatioTrace2 ... MinRatioTraceN }
my %MaxRatios = ();		# key: dimension	value: @{ MaxRatioTrace1, MaxRatioTrace2 ... MaxRatioTraceN }

for (my $i=0; $i<@Metrics; $i++)
{
	my $currentMetric = $Metrics[$i];
	for (my $currentTrace=0; $currentTrace<@InputTraces; $currentTrace++)
	{
		my $min_ratio = $arrayNumTasks[ $TraceWithMinDimension{$currentMetric} ] / $arrayNumTasks[$currentTrace];
		my $max_ratio = $arrayNumTasks[ $TraceWithMaxDimension{$currentMetric} ] / $arrayNumTasks[$currentTrace];
		push ( @{$MinRatios{$currentMetric}}, $min_ratio );
		push ( @{$MaxRatios{$currentMetric}}, $max_ratio );
	}
}

my %NormalMinDimensions = ();	# key: dimension	value: @{ NormalMinTrace1 ... NormalMinTraceN }
my %NormalMaxDimensions = ();	# key: dimension	value: @{ NormalMaxTrace1 ... NormalMaxTraceN }

for (my $i=0; $i<@Metrics; $i++)
{
    my $currentMetric = $Metrics[$i];
    for (my $currentTrace=0; $currentTrace<@InputTraces; $currentTrace++)
	{
		$normMin = @{$MinDimensionsPerTrace{$currentMetric}}[ $TraceWithMinDimension{$currentMetric} ] * 
			$MinRatios{$currentMetric}[$currentTrace];
		$normMax = @{$MaxDimensionsPerTrace{$currentMetric}}[ $TraceWithMaxDimension{$currentMetric} ] * 
			$MaxRatios{$currentMetric}[$currentTrace];
		push ( @{$NormalMinDimensions{$currentMetric}}, $normMin );
		push ( @{$NormalMaxDimensions{$currentMetric}}, $normMax );
	}
}

sub GetNormalizationType
{
	my ($Metric) = @_;

	if ( grep(/$Metric/, @MetricsFixScale) ) 
	{
		return $FIXED_SCALE;
	}
	else
	{
		return $SCALE_WITH_TASKS;
	}
}

sub CalcMinRange
{
    my ($Trace, $Metric) = @_;

    if (GetNormalizationType($Metric) == $FIXED_SCALE)
    {
        return $GlobalMinDimensions{$Metric};
    }
    else
    {
        return $NormalMinDimensions{$Metric}[$Trace];
    }
}

sub CalcMaxRange
{
	my ($Trace, $Metric) = @_;

	if (GetNormalizationType($Metric) == $FIXED_SCALE)
	{
		return $GlobalMaxDimensions{$Metric};
	}
	else
	{
		return $NormalMaxDimensions{$Metric}[$Trace];
	}
}

sub CalcNormalizedValue
{
	my ($Trace, $Metric, $Value) = @_;
	my $retVal;

	if (GetNormalizationType($Metric) == $FIXED_SCALE)
	{
		$retVal = 
			((log($Value) - log($GlobalMinDimensions{$Metric})) /
			 (log($GlobalMaxDimensions{$Metric}) - log($GlobalMinDimensions{$Metric})))
	}
	else
	{
		$retVal = 
			((log($Value) - log($MinDimensionsPerTrace{$Metric}[$Trace])) / 
			 (log($MaxDimensionsPerTrace{$Metric}[$Trace]) - log($MinDimensionsPerTrace{$Metric}[$Trace])));
	}
	return $retVal;
}

for ($currentTrace = 0; $currentTrace<@InputTraces; $currentTrace++)
{
    my $Tracefile = $InputTraces[$currentTrace];
    (my $withoutExt = $Tracefile) =~ s/\.[^.]+$//;
    my $PlotFile = $withoutExt.$PLOTSUFFIX;
    $PlotFile = `ls $PlotFile`;
    my $CSVFile = $withoutExt.$CSVSUFFIX;

    chomp $PlotFile;
    chomp $CSVFile;

	my @PlotFileTokens = split(/\./, $PlotFile);
	my $PrintDimX = $PlotFileTokens[@PlotFileTokens - 3];
	my $PrintDimY = $PlotFileTokens[@PlotFileTokens - 2];

    # Write the scaled plot
    $xrange = "set xrange [".CalcMinRange($currentTrace, $PrintDimX).":".CalcMaxRange($currentTrace, $PrintDimX)."]";
    $yrange = "set yrange [".CalcMinRange($currentTrace, $PrintDimY).":".CalcMaxRange($currentTrace, $PrintDimY)."]";

    my $OutputPlot = $PlotFile.".scaled";
    `echo $xrange > $OutputPlot`;
    `echo $yrange >> $OutputPlot`;
    `cat $PlotFile >> $OutputPlot`;

    # Rewrite the CSV file with normalized values
    my $OutputCSV = $CSVFile.".norm";
    open CSV_IN, "$CSVFile";
    open CSV_OUT, ">$OutputCSV";

    my $currentLine = 0;
    while (defined ($line = <CSV_IN>))
    {
		chomp $line;
        if ($currentLine == 0)
        {
			$line =~ s/\s+//;
            @Metrics = split(/,/, $line);
        }
        else
        {
            @MetricsValues = split (/,/, $line);

			for (my $j=0; $j<@Metrics; $j++)
			{
				my $currentMetric = $Metrics[$j];
				$MetricsValues[$j] = sprintf "%.6f", CalcNormalizedValue($currentTrace, $currentMetric, $MetricsValues[$j]);
			}

            $line = join(",", @MetricsValues);
        }
        print CSV_OUT "$line\n";
        $currentLine ++;
	}
}

