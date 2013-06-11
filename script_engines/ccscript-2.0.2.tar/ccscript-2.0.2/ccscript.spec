Summary: GNU ccScript - a GNU Common C++ framework for embedded scripting
Name: ccscript2
Version: 2.0.2
Release: 10
Group: Development/Libraries
URL: http://www.gnu.org/software/ccscript
Source: ftp://ftp.gnu.org/gnu/ccscript/ccscript-%{PACKAGE_VERSION}.tar.gz
Prefix: %{_prefix}
Copyright: GPL
BuildRoot: %{_tmppath}/ccscript-root
Packager: David Sugar <dyfet@ostel.com>
Requires: commoncpp2 >= 0.99.0

%description
The GNU ccScript package offers a class extensible threaded embedded
scripting engine for use with GNU Common C++.  This engine is also used in 
GNU Bayonne (the GNU telephony applicatiion server package) and other 
parts of GNUCOMM (the GNU telephony meta-project). This engine differs 
from traditional scripting systems in that it is used to script near 
real-time state-event systems through deterministic callback step 
execution rather than the linear and non-deterministic fashion of embedded 
script systems such as tcl, libguile, etc.

%prep
rm -rf $RPM_BUILD_ROOT

%setup -n ccscript-%{PACKAGE_VERSION}
./configure --prefix=%{_prefix}

%build
uname -a|grep SMP && make -j 2 || make

%install
mkdir -p $RPM_BUILD_ROOT/%{_mandir}/man3
make prefix=$RPM_BUILD_ROOT/%{_prefix} \
	mandir=$RPM_BUILD_ROOT/%{_mandir} install
make prefix=$RPM_BUILD_ROOT/%{_prefix} \
	mandir=$RPM_BUILD_ROOT/%{_mandir} man
strip $RPM_BUILD_ROOT/%{_prefix}/lib/libcc*.so.*

%files
%defattr(-,root,root,0755)
%doc AUTHORS COPYING NEWS README TODO ChangeLog
%{_prefix}/lib/libcc*
%{_prefix}/include/cc++2/cc++/script.h
%{_prefix}/bin/ccscript
%{_prefix}/lib/ccscript2
%{_mandir}/man3/*.3cc*

%clean
rm -rf $RPM_BUILD_ROOT

%post
/sbin/ldconfig -n /usr/lib

%postun
/sbin/ldconfig -n /usr/lib

