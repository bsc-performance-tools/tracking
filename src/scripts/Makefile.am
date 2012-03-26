
TRACKING_PREPROCESS = tracking-preprocess
TRACKING_LAUNCHER   = tracking
AUXILIARY_PERLS     = GeneratePlots.pl ParaverColors.pl Recoloring.pl ScaleNPlots.pl

HISTO3D_TEMPLATE    = CallersTemplate.cfg

bin_SCRIPTS = $(TRACKING_PREPROCESS) $(TRACKING_LAUNCHER) $(AUXILIARY_PERLS)
noinst_SCRIPTS = $(HISTO3D_TEMPLATE)

install-data-hook:
	mkdir -p ${prefix}/etc
	cp $(HISTO3D_TEMPLATE) ${prefix}/etc
	$(top_srcdir)/substitute $(SED) "@sub_TRACKING_PREFIX@" ${prefix} ${bindir}/$(TRACKING_PREPROCESS)
	$(top_srcdir)/substitute $(SED) "@sub_TRACKING_PREFIX@" ${prefix} ${bindir}/$(TRACKING_LAUNCHER)
	$(top_srcdir)/substitute $(SED) "@sub_PARAVER_HOME@" @PARAVER_HOME@ ${bindir}/$(TRACKING_LAUNCHER)
	$(top_srcdir)/substitute $(SED) "@sub_CLUSTERING_HOME@" @CLUSTERING_HOME@ ${bindir}/$(TRACKING_LAUNCHER)
	$(top_srcdir)/substitute $(SED) "@sub_ANN_HOME@" @ANN_HOME@ ${bindir}/$(TRACKING_LAUNCHER)
	$(top_srcdir)/substitute $(SED) "@sub_BOOST_LDPATH@" @BOOST_LDPATH@ ${bindir}/$(TRACKING_LAUNCHER)
	chmod u+x ${bindir}/$(TRACKING_PREPROCESS)
	chmod u+x ${bindir}/$(TRACKING_LAUNCHER)

