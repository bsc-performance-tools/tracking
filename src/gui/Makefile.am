
GUI_TEMPLATE = xtrack.in
GUI = $(GUI_TEMPLATE:.in=)

ICONS_DIR = icons

PARAVER_CFGS = clusterid.cfg

#bin_SCRIPTS = convexhull.py

EXTRA_DIST =      \
  $(GUI_TEMPLATE) \
  $(ICONS_DIR)    \
  $(PARAVER_CFGS)

install-data-hook:
	mkdir -p $(prefix)/bin
	mkdir -p $(prefix)/etc
	cp $(GUI_TEMPLATE) $(prefix)/bin/$(GUI)
	$(top_srcdir)/substitute $(SED) "@sub_PREFIX@" "${prefix}" $(prefix)/bin/$(GUI)
	chmod u+x $(prefix)/bin/$(GUI)
	cp -r $(ICONS_DIR) $(prefix)/etc
	cp $(PARAVER_CFGS) $(prefix)/etc
