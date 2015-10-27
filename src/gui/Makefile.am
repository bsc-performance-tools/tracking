
GUI_LAUNCHER_TEMPLATE = xtrack.in
GUI_TEMPLATE          = xtrack.py.in

GUI_LAUNCHER = $(GUI_LAUNCHER_TEMPLATE:.in=)
GUI          = $(GUI_TEMPLATE:.in=)

ICONS_DIR    = icons

PARAVER_CFGS = clusterid.cfg

dist_bin_SCRIPTS = callers_frame.py  data_manager.py  decorations.py  plotting_manager.py

EXTRA_DIST =               \
  $(GUI_LAUNCHER_TEMPLATE) \
  $(GUI_TEMPLATE)          \
  $(ICONS_DIR)             \
  $(PARAVER_CFGS)          

install-data-hook:
	mkdir -p $(prefix)/bin
	mkdir -p $(prefix)/etc
	cp $(GUI_LAUNCHER_TEMPLATE) $(prefix)/bin/$(GUI_LAUNCHER)
	$(top_srcdir)/substitute $(SED) "@sub_PREFIX@" "${prefix}" $(prefix)/bin/$(GUI_LAUNCHER)
	chmod a+x $(prefix)/bin/$(GUI_LAUNCHER)
	cp $(GUI_TEMPLATE) $(prefix)/bin/$(GUI)
	$(top_srcdir)/substitute $(SED) "@sub_PREFIX@" "${prefix}" $(prefix)/bin/$(GUI)
	chmod a+x $(prefix)/bin/$(GUI)
	cp -r $(ICONS_DIR) $(prefix)/etc
	cp $(PARAVER_CFGS) $(prefix)/etc
