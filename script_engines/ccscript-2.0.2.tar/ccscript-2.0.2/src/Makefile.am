# Copyright (C) 1999-2001 Open Source Telecom Corporation.
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

AUTOMAKE_OPTIONS = no-dependencies dist-shar
LT_VERSION=@LT_CCXX_VERSION@
LDFLAGS = @LDFLAGS@ $(LT_VERSION) -release $(LT_RELEASE)
CXXFLAGS = @CXXFLAGS@ -DSCRIPT_MODULE_PATH=\"$(pkglibdir)\"
LIBS = @GNULIBS@
ccxxincludedir=$(includedir)/cc++2/cc++
usedatadir = $(pkglibdir)
pkglibdir = $(libdir)/ccscript2

lib_LTLIBRARIES = libccscript2.la
bin_PROGRAMS = ccscript

if MODULES
usedata_PROGRAMS = url.pkg file.pkg time.pkg date.pkg string.pkg
endif

libccscript2_la_SOURCES = script.cpp interp.cpp locks.cpp module.cpp

ccxxinclude_HEADERS = script.h

ccscript_SOURCES = shell.cpp
ccscript_LDFLAGS = libccscript2.la

if	MODULES
url_pkg_SOURCES = url.cpp
url_pkg_LDFLAGS = -module -shared $(EXTLIBS)

file_pkg_SOURCES = file.cpp
file_pkg_LDFLAGS = -module -shared

time_pkg_SOURCES = time.cpp
time_pkg_LDFLAGS = -module -shared

date_pkg_SOURCES = date.cpp
date_pkg_LDFLAGS = -module -shared

string_pkg_SOURCES = string.cpp
string_pkg_LDFLAGS = -module -shared
endif

kdoc_headers = script.h

kdoc:
	kdoc -f html -d $(KDOC_DIR) -L$(KDOC_DIR) -n ostscript $(kdoc_headers)



