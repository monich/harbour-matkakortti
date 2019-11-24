Name:           harbour-matkakortti
Summary:        Application to read HSL travel card
Version:        1.0.1
Release:        1
License:        BSD
Group:          Applications/System
URL:            https://github.com/monich/harbour-matkakortti
Source0:        %{name}-%{version}.tar.gz

Requires:       sailfishsilica-qt5
Requires:       qt5-qtsvg-plugin-imageformat-svg
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(sailfishapp)

%define qt_version 5.2

BuildRequires:  pkgconfig(Qt5Core) >= %{qt_version}
BuildRequires:  pkgconfig(Qt5Qml) >= %{qt_version}
BuildRequires:  pkgconfig(Qt5Quick) >= %{qt_version}
BuildRequires:  qt5-qttools-linguist
Requires: qt5-qtcore >= %{qt_version}

%{!?qtc_qmake5:%define qtc_qmake5 %qmake5}
%{!?qtc_make:%define qtc_make make}
%{?qtc_builddir:%define _builddir %qtc_builddir}

%description
Demonstrates use of NFC in Sailfish OS.

%prep
%setup -q -n %{name}-%{version}

%build
%qtc_qmake5 %{name}.pro
%qtc_make %{?_smp_mflags}

%install
rm -rf %{buildroot}
%qmake5_install

desktop-file-install --delete-original \
  --dir %{buildroot}%{_datadir}/applications \
   %{buildroot}%{_datadir}/applications/*.desktop

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png
