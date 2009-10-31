# what flags you want to pass to the C compiler & linker
AM_CFLAGS = 	-Wall -W -O2 \
  @GTK_CFLAGS@ \
  @GSTREAMER_CFLAGS@ \
  @TAGLIB_CFLAGS@ \
  $(TABU_DEFINES)
AM_LDFLAGS =	@GTK_LIBS@ \
  @GSTREAMER_LIBS@ \
  @TAGLIB_LIBS@

# this lists the binaries to produce, the (non-PHONY, binary) targets in
# the previous manual Makefile
bin_PROGRAMS = tabu
tabu_SOURCES = tabu.c tabu.h \
  tabu-window.c tabu-window.h \
  tabu-controls.c tabu-controls.h \
  tabu-playlist.c tabu-playlist.h \
  tabu-backend.c tabu-backend.h

desktopdir = $(datadir)/applications
desktop_DATA = tabu.desktop

pixmapdir = $(datadir)/tabu
pixmap_DATA = images/*

EXTRA_DIST = $(desktop_DATA) $(pixmap_DATA)
