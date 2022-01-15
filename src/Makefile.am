## Process this file with automake to produce Makefile.in
plugindir=$(prefix)/lib/cssed/plugins
plugin_PROGRAMS=tagsplugin.so

SRCS = \
	tagplugin.c tagplugin.h \
	interface.c interface.h \
	callbacks.c callbacks.h \
	support.c support.h \
	readtags.c readtags.h

if ISDARWIN
tagsplugin_so_SOURCES = $(SRCS)
tagsplugin_so_LDFLAGS =  -bundle -flat_namespace -undefined suppress
tagsplugin_so_LDADD = @PACKAGE_LIBS@
tagsplugin_so_CFLAGS = -fno-common
else
tagsplugin_so_SOURCES= $(SRCS)
tagsplugin_so_LDFLAGS = -shared -fPICS
tagsplugin_so_LDADD = @PACKAGE_LIBS@
endif


INCLUDES = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/share/locale"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LIB_DIR=\""$(prefix)/lib/cssed/"\" \
	-I$(top_srcdir) -I$(top_srcdir)/src \
	@PACKAGE_CFLAGS@



