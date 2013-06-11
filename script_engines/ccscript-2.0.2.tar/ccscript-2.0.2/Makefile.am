# Copyright (C) 2001 Open Source Telecom Corporation.
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

AUTOMAKE_OPTIONS = no-dependencies dist-shar
EXTRA_DIST = reconfig ccscript.spec ccscript.spec.in ccscript.proto \
	pkginfo.in freebsd/Makefile.in freebsd/pkg-*

PKGBASE = "/opt/OSTscript"
PKGNAME = "OSTscript"

SUBDIRS = src
DIST_SUBDIRS = m4 config src doc

kdoc:
	(cd src ; make kdoc)

doxy:
	(cd doc ; doxygen Doxyfile)
	(cd doc ; doxygen Doxyman)

man:
	(cd doc/man3 ; $(INSTALL_DATA) *.3cc $(mandir)/man3 )

pkg:	
	$(MAKE) $(AM_MAKEFLAGS) prefix=$(PKGBASE) install
	pkgmk -b  $(PKGBASE) -f $(PACKAGE).proto
	pkgtrans -s /var/spool/pkg $(PKGNAME)-$(host_alias).pkg $(PKGNAME)
	mv /var/spool/pkg/$(PKGNAME)-*.pkg . 
	rm -rf $(PKGBASE)
	rm -rf /var/spool/pkg/$(PKGNAME)



lsm:	$(distdir).tar.gz
	sed -e s/SIZE/$(shell expr `wc -c *.tar.gz | cut -d\  -f2` / 1024)/ -e s/VERSION/$(VERSION)/ -e s/DATE/$(shell date +%Y%m%d)/ <$(top_srcdir)/ccscript.lsm.in >ccscript.lsm

zip:
	zip -r9 ccscript-$(VERSION).zip src/*.cpp src/*.h README ChangeLog AUTHORS NEWS TODO

ports:
	mkdir -p freebsd/files
	(cd freebsd ; shar Makefile pkg-descr pkg-comment pkg-plist > ../$(distdir).ports )



