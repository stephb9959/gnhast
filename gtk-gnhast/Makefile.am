## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = -DLOCALSTATEDIR=\"$(localstatedir)\" \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-I$(top_srcdir)/common \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	@GTK2_CFLAGS@

bin_PROGRAMS = gtk-gnhast

gtk_gnhast_SOURCES = \
        $(top_srcdir)/common/genconn.h \
        $(top_srcdir)/common/common.h \
        $(top_srcdir)/common/gnhast.h \
        $(top_srcdir)/common/confuse.h \
        $(top_srcdir)/common/confparser.h \
        $(top_srcdir)/common/gncoll.h \
        $(top_srcdir)/common/collcmd.h \
	main.c \
	gtk-gnhast.h collector.h \
	support.c support.h \
	interface.c interface.h \
	callbacks.c callbacks.h

if NEED_RBTREE
gtk_gnhast_SOURCES += \
        $(top_srcdir)/linux/queue.h \
        $(top_srcdir)/linux/endian.h \
        $(top_srcdir)/linux/rbtree.h \
        $(top_srcdir)/linux/time.h
endif

gtk_gnhast_LDADD = @GTK2_LIBS@ \
	$(top_builddir)/libconfuse/libgnconfuse.la \
	$(top_builddir)/common/libgnhast.la
