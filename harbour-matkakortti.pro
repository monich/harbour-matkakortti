NAME = matkakortti
PREFIX = harbour

TARGET = $${PREFIX}-$${NAME}
CONFIG += sailfishapp link_pkgconfig
PKGCONFIG += sailfishapp glib-2.0 gobject-2.0 gio-unix-2.0
QT += qml quick dbus

QMAKE_CXXFLAGS += -Wno-unused-parameter -Wno-psabi
QMAKE_CFLAGS += -Wno-unused-parameter
LIBS += -ldl

TARGET_DATA_DIR = /usr/share/$${TARGET}

app_settings {
    # This path is hardcoded in jolla-settings
    TRANSLATIONS_PATH = /usr/share/translations
} else {
    TRANSLATIONS_PATH = $${TARGET_DATA_DIR}/translations
}

CONFIG(debug, debug|release) {
    DEFINES += DEBUG HARBOUR_DEBUG
}

# Directories

HARBOUR_LIB_DIR = $${_PRO_FILE_PWD_}/harbour-lib
LIBGLIBUTIL_DIR = $${_PRO_FILE_PWD_}/libglibutil
LIBGNFCDC_DIR = $${_PRO_FILE_PWD_}/libgnfcdc

# Files

OTHER_FILES += \
    LICENSE \
    README.md \
    rpm/*.spec \
    *.desktop \
    qml/*.qml \
    qml/images/*.svg \
    qml/components/*.js \
    qml/components/*.qml \
    icons/*.svg \
    translations/*.ts

# libglibutil

LIBGLIBUTIL_SRC = $${LIBGLIBUTIL_DIR}/src
LIBGLIBUTIL_INCLUDE = $${LIBGLIBUTIL_DIR}/include

INCLUDEPATH += \
    $${LIBGLIBUTIL_INCLUDE}

HEADERS += \
    $${LIBGLIBUTIL_INCLUDE}/*.h

SOURCES += \
    $${LIBGLIBUTIL_SRC}/gutil_log.c \
    $${LIBGLIBUTIL_SRC}/gutil_misc.c \
    $${LIBGLIBUTIL_SRC}/gutil_strv.c \
    $${LIBGLIBUTIL_SRC}/gutil_timenotify.c

# libgnfcdc

LIBGNFCDC_INCLUDE = $${LIBGNFCDC_DIR}/include
LIBGNFCDC_SRC = $${LIBGNFCDC_DIR}/src
LIBGNFCDC_SPEC = $${LIBGNFCDC_DIR}/spec

INCLUDEPATH += \
    $${LIBGNFCDC_INCLUDE}

HEADERS += \
    $${LIBGNFCDC_INCLUDE}/*.h \
    $${LIBGNFCDC_SRC}/*.h

SOURCES += \
    $${LIBGNFCDC_SRC}/*.c

OTHER_FILES += \
    $${LIBGNFCDC_SPEC}/*.xml

defineTest(generateStub) {
    xml = $${LIBGNFCDC_SPEC}/org.sailfishos.nfc.$${1}.xml
    cmd = gdbus-codegen --generate-c-code org.sailfishos.nfc.$${1} $${xml}

    file = org.sailfishos.nfc.$${1}.c
    target = org_sailfishos_nfc_$${1}

    $${target}.target = $${file}
    $${target}.depends = $${xml}
    $${target}.commands = $${cmd}
    export($${target}.target)
    export($${target}.depends)
    export($${target}.commands)

    QMAKE_EXTRA_TARGETS += $${target}
    GENERATED_SOURCES += $${file}
    PRE_TARGETDEPS += $${file}

    export(QMAKE_EXTRA_TARGETS)
    export(GENERATED_SOURCES)
    export(PRE_TARGETDEPS)
}

generateStub(Adapter)
generateStub(Daemon)
generateStub(IsoDep)
generateStub(Settings)
generateStub(Tag)

# harbour-lib

HARBOUR_LIB_INCLUDE = $${HARBOUR_LIB_DIR}/include
HARBOUR_LIB_SRC = $${HARBOUR_LIB_DIR}/src
HARBOUR_LIB_QML = $${HARBOUR_LIB_DIR}/qml

INCLUDEPATH += \
    $${HARBOUR_LIB_INCLUDE}

HEADERS += \
    $${HARBOUR_LIB_INCLUDE}/HarbourDebug.h \
    $${HARBOUR_LIB_INCLUDE}/HarbourSystemTime.h \
    $${HARBOUR_LIB_INCLUDE}/HarbourSystem.h \
    $${HARBOUR_LIB_INCLUDE}/HarbourTheme.h

SOURCES += \
    $${HARBOUR_LIB_SRC}/HarbourSystemTime.cpp \
    $${HARBOUR_LIB_SRC}/HarbourSystem.cpp \
    $${HARBOUR_LIB_SRC}/HarbourTheme.cpp

HARBOUR_QML_COMPONENTS = \
    $${HARBOUR_LIB_QML}/HarbourHighlightIcon.qml

OTHER_FILES += $${HARBOUR_QML_COMPONENTS}

qml_components.files = $${HARBOUR_QML_COMPONENTS}
qml_components.path = $${TARGET_DATA_DIR}/qml/harbour
INSTALLS += qml_components

# App

INCLUDEPATH += \
    src

HEADERS += \
    src/NfcAdapter.h \
    src/NfcSystem.h \
    src/NfcTag.h \
    src/TravelCard.h \
    src/TravelCardImpl.h \
    src/Util.h

SOURCES += \
    src/main.cpp \
    src/NfcAdapter.cpp \
    src/NfcSystem.cpp \
    src/NfcTag.cpp \
    src/TravelCard.cpp \
    src/Util.cpp

# HSL

OTHER_FILES += \
    qml/hsl/*.qml \
    qml/hsl/images/*.svg

HEADERS += \
    src/hsl/HslArea.h \
    src/hsl/HslCard.h \
    src/hsl/HslCardAppInfo.h \
    src/hsl/HslCardEticket.h \
    src/hsl/HslCardHistory.h \
    src/hsl/HslCardPeriodPass.h \
    src/hsl/HslCardStoredValue.h \
    src/hsl/HslData.h

SOURCES += \
    src/hsl/HslArea.cpp \
    src/hsl/HslCard.cpp \
    src/hsl/HslCardAppInfo.cpp \
    src/hsl/HslCardEticket.cpp \
    src/hsl/HslCardHistory.cpp \
    src/hsl/HslCardPeriodPass.cpp \
    src/hsl/HslCardStoredValue.cpp \
    src/hsl/HslData.cpp

# Nysse

OTHER_FILES += \
    qml/nysse/*.qml \
    qml/nysse/images/*.svg

HEADERS += \
    src/nysse/NysseCard.h \
    src/nysse/NysseCardAppInfo.h \
    src/nysse/NysseCardBalance.h \
    src/nysse/NysseCardHistory.h \
    src/nysse/NysseCardOwnerInfo.h \
    src/nysse/NysseCardSeasonPass.h \
    src/nysse/NysseUtil.h

SOURCES += \
    src/nysse/NysseCard.cpp \
    src/nysse/NysseCardAppInfo.cpp \
    src/nysse/NysseCardBalance.cpp \
    src/nysse/NysseCardHistory.cpp \
    src/nysse/NysseCardOwnerInfo.cpp \
    src/nysse/NysseCardSeasonPass.cpp \
    src/nysse/NysseUtil.cpp

# Icons
ICON_SIZES = 86 108 128 256
for(s, ICON_SIZES) {
    icon_target = icon_$${s}
    icon_dir = icons/$${s}x$${s}
    $${icon_target}.files = $${icon_dir}/$${TARGET}.png
    $${icon_target}.path = /usr/share/icons/hicolor/$${s}x$${s}/apps
    INSTALLS += $${icon_target}
}

# Translations
TRANSLATION_SOURCES = \
  $${_PRO_FILE_PWD_}/qml

defineTest(addTrFile) {
    in = $${_PRO_FILE_PWD_}/translations/harbour-$$1
    out = $${OUT_PWD}/translations/$${PREFIX}-$$1

    s = $$replace(1,-,_)
    lupdate_target = lupdate_$$s
    lrelease_target = lrelease_$$s

    $${lupdate_target}.commands = lupdate -noobsolete -locations none $${TRANSLATION_SOURCES} -ts \"$${in}.ts\" && \
        mkdir -p \"$${OUT_PWD}/translations\" &&  [ \"$${in}.ts\" != \"$${out}.ts\" ] && \
        cp -af \"$${in}.ts\" \"$${out}.ts\" || :

    $${lrelease_target}.target = $${out}.qm
    $${lrelease_target}.depends = $${lupdate_target}
    $${lrelease_target}.commands = lrelease -idbased \"$${out}.ts\"

    QMAKE_EXTRA_TARGETS += $${lrelease_target} $${lupdate_target}
    PRE_TARGETDEPS += $${out}.qm
    qm.files += $${out}.qm

    export($${lupdate_target}.commands)
    export($${lrelease_target}.target)
    export($${lrelease_target}.depends)
    export($${lrelease_target}.commands)
    export(QMAKE_EXTRA_TARGETS)
    export(PRE_TARGETDEPS)
    export(qm.files)
}

LANGUAGES = fi pl ru sv zh_CN

addTrFile($${NAME})
for(l, LANGUAGES) {
    addTrFile($${NAME}-$$l)
}

qm.path = $$TRANSLATIONS_PATH
qm.CONFIG += no_check_exist
INSTALLS += qm

OTHER_FILES += LICENSE
