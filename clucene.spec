Summary:	A C++ port of Lucene
Name:		clucene
Version:	2.3.3.4
Release:	11%{?dist}
License:	LGPLv2+ or ASL 2.0
Group:		Development/System
URL:		http://www.sourceforge.net/projects/clucene
Source0:	http://downloads.sourceforge.net/clucene/clucene-core-%{version}.tar.gz
BuildRequires:	gawk cmake zlib-devel boost-devel

## upstreamable patches
# include LUCENE_SYS_INCLUDES in pkgconfig --cflags output
# https://bugzilla.redhat.com/748196
# and
# https://sourceforge.net/tracker/?func=detail&aid=3461512&group_id=80013&atid=558446
# pkgconfig file is missing clucene-shared
Patch50: clucene-core-2.3.3.4-pkgconfig.patch
# https://bugzilla.redhat.com/794795
# https://sourceforge.net/tracker/index.php?func=detail&aid=3392466&group_id=80013&atid=558446
# contribs-lib is not built and installed even with config
Patch51: clucene-core-2.3.3.4-install_contribs_lib.patch
# Bug 1027836 - clucene breaks rebuild of libreoffice
Patch52: clucene-core-2.3.3.4-lo.patch

%description
CLucene is a C++ port of the popular Apache Lucene search engine
(http://lucene.apache.org/java). 
CLucene aims to be a high-speed alternative to Java Lucene, its API is very
similar to that of the Java version. CLucene has recently been brought up to
date with Lucene 2.3.2. It contains most of the same functionality as the Java version.

%package core
Summary:	Core clucene module
Group:		Development/System
Provides:	clucene = %{version}-%{release}
#Requires: %{name} = %{version}-%{release}
%description core
CLucene is a C++ port of the popular Apache Lucene search engine
(http://lucene.apache.org/java).
CLucene aims to be a high-speed alternative to Java Lucene, its API is very
similar to that of the Java version. CLucene has recently been brought up to
date with Lucene 2.3.2. It contains most of the same functionality as the Java version.

%package core-devel
Summary:	Headers for developing programs that will use %{name}
Group:		Development/Libraries
Requires:	%{name}-core%{?_isa} = %{version}-%{release}
Requires:	%{name}-contribs-lib%{?_isa} = %{version}-%{release}
%description core-devel
This package contains the libraries and header files needed for
developing with clucene

%package contribs-lib
Summary:	Language specific text analyzers for %{name}
Group:  	Development/System
Requires:	%{name}-core%{?_isa} = %{version}-%{release}
%description contribs-lib
%{summary}.


%prep
%setup -n %{name}-core-%{version}

%patch50 -p1 -b .pkgconfig
%patch51 -p1 -b .install_contribs_lib
%patch52 -p0 -b .libreoffice

%build
mkdir -p %{_target_platform}
pushd %{_target_platform}
%{cmake} \
  -DBUILD_CONTRIBS_LIB=BOOL:ON \
  -DLIB_DESTINATION:PATH=%{_libdir} \
  -DLUCENE_SYS_INCLUDES:PATH=%{_libdir} \
  ..
popd

make %{?_smp_mflags} -C %{_target_platform}


%install
make install/fast DESTDIR=%{buildroot} -C %{_target_platform}

rm -rf %{buildroot}%{_libdir}/CLuceneConfig.cmake


%check
# FIXME: do not run tests for ppc and s390 (big endian 32 bit archs) until
# we have a proper fix
%ifnarch ppc s390
# Fails on all arches at the moment so temporaily disable
#make cl_test -C %{_target_platform}
#make test -C %{_target_platform}
%endif


%post core -p /sbin/ldconfig
%postun core -p /sbin/ldconfig

%files core
%defattr(-, root, root, -)
%doc APACHE.license AUTHORS ChangeLog COPYING LGPL.license README
%{_libdir}/libclucene-core.so.1*
%{_libdir}/libclucene-core.so.%{version}
%{_libdir}/libclucene-shared.so.1*
%{_libdir}/libclucene-shared.so.%{version}

%post contribs-lib -p /sbin/ldconfig
%postun contribs-lib -p /sbin/ldconfig

%files contribs-lib
%defattr(-, root, root, -)
%{_libdir}/libclucene-contribs-lib.so.1*
%{_libdir}/libclucene-contribs-lib.so.%{version}

%files core-devel
%defattr(-, root, root, -)
%dir %{_libdir}/CLucene
%{_includedir}/CLucene/
%{_includedir}/CLucene.h
%{_libdir}/libclucene*.so
%{_libdir}/CLucene/clucene-config.h
%{_libdir}/CLucene/CLuceneConfig.cmake
%{_libdir}/pkgconfig/libclucene-core.pc


%changelog
* Fri Jan 24 2014 Daniel Mach <dmach@redhat.com> - 2.3.3.4-11
- Mass rebuild 2014-01-24

* Fri Dec 27 2013 Daniel Mach <dmach@redhat.com> - 2.3.3.4-10
- Mass rebuild 2013-12-27

* Thu Nov 7 2013 Lukáš Tinkl <ltinkl@redhat.com> - 2.3.3.4-9
- Resolves: rhbz#1027836 - clucene breaks rebuild of libreoffice

* Wed Feb 13 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.3.3.4-8
- Rebuilt for https://fedoraproject.org/wiki/Fedora_19_Mass_Rebuild

* Wed Jul 18 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.3.3.4-7
- Rebuilt for https://fedoraproject.org/wiki/Fedora_18_Mass_Rebuild

* Mon Mar 19 2012 Rex Dieter <rdieter@fedoraproject.org> 2.3.3.4-6
- contribs-lib is not built and installed even with config (#794795, upstream ID: 3392466)
- pkgconfig file is missing clucene-shared (upstream ID: 3461512)
- non-descriptive descripton (#757319)

* Sat Feb 25 2012 Peter Robinson <pbrobinson@fedoraproject.org> - 2.3.3.4-5
- Temporarily disable make check as it fails on all arches

* Thu Jan 12 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.3.3.4-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_17_Mass_Rebuild

* Sun Oct 23 2011 Rex Dieter <rdieter@fedoraproject.org> 2.3.3.4-3
- include LUCENE_SYS_INCLUDES in pkgconfig --cflags output (#748196)

* Wed Jun 08 2011 Rex Dieter <rdieter@fedoraproject.org> 2.3.3.4-2
- cleanup cmake usage
- fix scriptlets
- track sonames

* Thu Jun 02 2011 Deji Akingunola <dakingun@gmail.com> - 2.3.3.4-1
- Update to version 2.3.3.4

* Tue Feb 08 2011 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.9.21b-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_15_Mass_Rebuild

* Thu Jul 08 2010 Deji Akingunola <dakingun@gmail.com> 0.9.21b-2
- Include the license text in the -core subpackage.

* Sun Jun 06 2010 Robert Scheck <robert@fedoraproject.org> 0.9.21b-1
- Update to 0.9.21b

* Wed Nov 04 2009 Dennis Gilmore <dennis@ausil.us> - 0.9.21-5
- disable 'make check on sparc64 along with ppc64 and s390x

* Fri Jul 24 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.9.21-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_12_Mass_Rebuild

* Tue Apr 14 2009 Karsten Hopp <karsten@redhat.com> 0.9.21-3
- bypass 'make check' on s390x, similar to ppc64

* Tue Feb 24 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.9.21-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_11_Mass_Rebuild

* Wed Aug 27 2008 Deji Akingunola <dakingun@gmail.com> - 0.9.21-1
- Update to version 0.9.21

* Sun Feb 10 2008 Deji Akingunola <dakingun@gmail.com> - 0.9.20-4
- Rebuild for gcc43

* Wed Oct 25 2007 Deji Akingunola <dakingun@gmail.com> - 0.9.20-3
- Fix a typo in the License field

* Wed Oct 25 2007 Deji Akingunola <dakingun@gmail.com> - 0.9.20-2
- Fix multiarch conflicts (BZ #340891)
- Bypass 'make check' for ppc64, its failing two tests there

* Tue Aug 21 2007 Deji Akingunola <dakingun@gmail.com> - 0.9.20-1
- Update to version 0.9.20

* Sat Aug 11 2007 Deji Akingunola <dakingun@gmail.com> - 0.9.19-1
- Latest release update

* Fri Aug 03 2007 Deji Akingunola <dakingun@gmail.com> - 0.9.16a-2
- License tag update

* Thu Feb 22 2007 Deji Akingunola <dakingun@gmail.com> - 0.9.16a-2
- Add -contrib subpackage 

* Thu Dec 07 2006 Deji Akingunola <dakingun@gmail.com> - 0.9.16a-1
- Update to latest stable release 
- Run make check during build

* Mon Nov 20 2006 Deji Akingunola <dakingun@gmail.com> - 0.9.15-3
- Don't package APACHE.license since we've LGPL instead 
- Package documentation in devel subpackage

* Mon Nov 13 2006 Deji Akingunola <dakingun@gmail.com> - 0.9.15-2
- Fix a bunch of issues with the spec (#215258)
- Moved the header file away from lib dir

* Sat Nov 04 2006 Deji Akingunola <dakingun@gmail.com> - 0.9.15-1
- Initial packaging for Fedora Extras
