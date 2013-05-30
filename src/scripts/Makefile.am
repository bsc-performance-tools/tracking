
TRACKING_PREPROCESS   = tracking.pl
TRACKING_LAUNCHER     = tracking
SCRIPT_SCALE_FRAMES   = scale-frames.py
SCRIPT_GENERATE_PLOTS = generate-plots.pl
SCRIPT_COLOR_PALETTE  = color-palette.pl
SCRIPT_RECOLORING     = recoloring.pl
AUXILIARIES           = $(SCRIPT_SCALE_FRAMES) $(SCRIPT_GENERATE_PLOTS) $(SCRIPT_COLOR_PALETTE) $(SCRIPT_RECOLORING)

HISTO3D_CALLERS_TEMPLATE = callers-template.cfg

bin_SCRIPTS = $(TRACKING_PREPROCESS) $(TRACKING_LAUNCHER) $(AUXILIARIES)
noinst_SCRIPTS = $(HISTO3D_CALLERS_TEMPLATE)

install-data-hook:
	mkdir -p ${prefix}/etc
	cp $(HISTO3D_CALLERS_TEMPLATE) ${prefix}/etc
	$(top_srcdir)/substitute $(SED) "@sub_TRACKING_PREFIX@" ${prefix} ${bindir}/$(TRACKING_PREPROCESS)
	$(top_srcdir)/substitute $(SED) "@sub_TRACKING_PREFIX@" ${prefix} ${bindir}/$(TRACKING_LAUNCHER)
	$(top_srcdir)/substitute $(SED) "@sub_BOOST_LDPATH@" @BOOST_LDPATH@ ${bindir}/$(TRACKING_LAUNCHER)
	$(top_srcdir)/substitute $(SED) "@sub_CLUSTERING_HOME@" @CLUSTERING_HOME@ ${bindir}/$(TRACKING_PREPROCESS)
	$(top_srcdir)/substitute $(SED) "@sub_SCRIPT_SCALE_FRAMES@" $(SCRIPT_SCALE_FRAMES) ${bindir}/$(TRACKING_PREPROCESS)
	$(top_srcdir)/substitute $(SED) "@sub_CALLERS_TEMPLATE@" $(HISTO3D_CALLERS_TEMPLATE) ${bindir}/$(TRACKING_PREPROCESS)
	chmod u+x ${bindir}/$(TRACKING_PREPROCESS)
	chmod u+x ${bindir}/$(TRACKING_LAUNCHER)

