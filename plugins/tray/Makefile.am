INCLUDES =					\
	-I$(top_builddir)			\
	-I$(top_srcdir)				\
	-DG_LOG_DOMAIN=\"parole_tray\"    	\
	-DLIBEXECDIR=\"$(libexecdir)\"		\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"

pluginsdir = 					\
	$(libdir)/parole-$(PAROLE_VERSION_API)

plugins_LTLIBRARIES =				\
	tray-icon.la

tray_icon_la_SOURCES =				\
	tray-plugin.c				\
	tray-provider.c				\
	tray-provider.h

tray_icon_la_CFLAGS =				\
	$(PLATFORM_CFLAGS)			\
	$(GTK_CFLAGS)				\
	$(LIBXFCE4GUI_CFLAGS)			\
	$(LIBXFCE4UTIL_CFLAGS)			\
	$(LIBNOTIFY_CFLAGS)

tray_icon_la_LDFLAGS =				\
	-avoid-version				\
	-export-dynamic				\
	-module					\
	$(PLATFORM_LDFLAGS)

tray_icon_la_LIBADD =				\
	$(LIBNOTIFY_LIBS)

#
# .desktop file
#
desktop_in_files = system-tray.desktop.in
desktopdir = $(datadir)/parole/parole-plugins-$(PAROLE_VERSION_API)
desktop_DATA =  $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = 		  	\
	$(desktop_in_files)
    
DISTCLEANFILES = 		\
	$(desktop_DATA)