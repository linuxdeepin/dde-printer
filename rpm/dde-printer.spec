%define pkgrelease  1
%if 0%{?openeuler}
%define specrelease %{pkgrelease}
%else
## allow specrelease to have configurable %%{?dist} tag in other distribution
%define specrelease %{pkgrelease}%{?dist}
%endif

Name:       dde-printer
Version:    0.7.11
Release:    %{specrelease}
Summary:    Printer Manager is tool to manage printers
License:    GPLv3
URL:        https://github.com/linuxdeepin/%{name}
Packager:   LiuRui
Source0:    %{name}_%{version}.orig.tar.xz

BuildRequires:  gcc-c++
BuildRequires:  qt5-devel
BuildRequires:  cups-devel
BuildRequires:  dtkwidget-devel
BuildRequires:  dtkcore-devel >= 5.2.2.3
BuildRequires:  qt5-qttools
BuildRequires:  gnutls-devel
BuildRequires:  libsmbclient-devel
BuildRequires:  qt5-linguist
BuildRequires:  qt5-qtbase-devel

Requires: qt5-devel
Requires: cups-libs
Requires: dtkwidget
Requires: dtkcore >= 5.2.2.3,
Requires: gnutls
Requires: libsmbclient


%description
%{summary}.


%prep
%autosetup

%build
# help find (and prefer) qt5 utilities, e.g. qmake, lrelease
export PATH=%{_qt5_bindir}:$PATH
mkdir build && pushd build
%qmake_qt5 ../
%make_build
popd

%install
%make_install -C build INSTALL_ROOT="%buildroot"

%posttrans
if [ -n "$(ps -ef | grep dde-printer | grep -v grep)" ];then 
    killall -USR1 dde-printer-helper
fi

%files
%doc README.md
%license LICENSE
%{_bindir}/%{name}
%{_datadir}/icons/hicolor/48x48/apps/dde-printer.svg
%{_datadir}/%{name}/translations/*.qm
%{_datadir}/applications/%{name}.desktop
%{_datadir}/polkit-1/actions/com.deepin.pkexec.devPrinter.policy

%{_bindir}/dde-printer-helper
%{_datadir}/dde-printer-helper/translations/*.qm
/etc/xdg/autostart/dde-printer-watch.desktop
%{_datadir}/deepin-manual/manual-assets/application

%changelog

