Name:           harbour-matkakortti
Summary:        Application for reading HSL and Tampere travel cards
Version:        1.1.4
Release:        1
License:        BSD
Group:          Applications/System
URL:            https://github.com/monich/harbour-matkakortti
Source0:        %{name}-%{version}.tar.gz

Requires:       sailfishsilica-qt5
Requires:       qt5-qtsvg-plugin-imageformat-svg
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(sailfishapp)

BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  qt5-qttools-linguist

%{!?qtc_qmake5:%define qtc_qmake5 %qmake5}
%{!?qtc_make:%define qtc_make make}
%{?qtc_builddir:%define _builddir %qtc_builddir}

%description
Reads information off Helsinki and Tampere travel cards via NFC.

%if "%{?vendor}" == "chum"
Categories:
 - Utility
Icon: https://raw.githubusercontent.com/monich/harbour-matkakortti/master/icons/harbour-matkakortti.svg
Screenshots:
- https://home.monich.net/chum/harbour-matkakortti/screenshots/screenshot-001.png
- https://home.monich.net/chum/harbour-matkakortti/screenshots/screenshot-002.png
- https://home.monich.net/chum/harbour-matkakortti/screenshots/screenshot-003.png
- https://home.monich.net/chum/harbour-matkakortti/screenshots/screenshot-004.png
- https://home.monich.net/chum/harbour-matkakortti/screenshots/screenshot-005.png
- https://home.monich.net/chum/harbour-matkakortti/screenshots/screenshot-006.png
Url:
  Homepage: https://openrepos.net/content/slava/matkakortti
%endif

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
