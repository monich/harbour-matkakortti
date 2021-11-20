NAME = matkakortti
PREFIX = harbour

TARGET = $${PREFIX}-$${NAME}
CONFIG += sailfishapp link_pkgconfig
PKGCONFIG += sailfishapp glib-2.0 gobject-2.0 gio-unix-2.0
QT += qml quick dbus

DEFINES += NFCDC_NEED_PEER_SERVICE=0
QMAKE_CXXFLAGS += -Wno-unused-parameter
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
} else {
    QMAKE_CXXFLAGS += -flto -fPIC
    QMAKE_CFLAGS += -flto -fPIC
    QMAKE_LFLAGS += -flto -fPIC
}

# Directories

HARBOUR_LIB_DIR = $${_PRO_FILE_PWD_}/harbour-lib
LIBGLIBUTIL_DIR = $${_PRO_FILE_PWD_}/libglibutil
LIBGNFCDC_DIR = $${_PRO_FILE_PWD_}/libgnfcdc
LIBQNFCDC_DIR = $${_PRO_FILE_PWD_}/libqnfcdc

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
    $${LIBGNFCDC_SRC}/nfcdc_adapter.c \
    $${LIBGNFCDC_SRC}/nfcdc_base.c \
    $${LIBGNFCDC_SRC}/nfcdc_daemon.c \
    $${LIBGNFCDC_SRC}/nfcdc_default_adapter.c \
    $${LIBGNFCDC_SRC}/nfcdc_error.c \
    $${LIBGNFCDC_SRC}/nfcdc_isodep.c \
    $${LIBGNFCDC_SRC}/nfcdc_log.c \
    $${LIBGNFCDC_SRC}/nfcdc_tag.c

OTHER_FILES += \
    $${LIBGNFCDC_SPEC}/*.xml

defineTest(generateStub) {
    xml = $${LIBGNFCDC_SPEC}/org.sailfishos.nfc.$${1}.xml
    cmd = gdbus-codegen --generate-c-code org.sailfishos.nfc.$${1} $${xml}

    gen_h = org.sailfishos.nfc.$${1}.h
    gen_c = org.sailfishos.nfc.$${1}.c
    target_h = org_sailfishos_nfc_$${1}_h
    target_c = org_sailfishos_nfc_$${1}_c

    $${target_h}.target = $${gen_h}
    $${target_h}.depends = $${xml}
    $${target_h}.commands = $${cmd}
    export($${target_h}.target)
    export($${target_h}.depends)
    export($${target_h}.commands)

    GENERATED_HEADERS += $${gen_h}
    PRE_TARGETDEPS += $${gen_h}
    QMAKE_EXTRA_TARGETS += $${target_h}

    $${target_c}.target = $${gen_c}
    $${target_c}.depends = $${gen_h}
    export($${target_c}.target)
    export($${target_c}.depends)

    GENERATED_SOURCES += $${gen_c}
    QMAKE_EXTRA_TARGETS += $${target_c}
    PRE_TARGETDEPS += $${gen_c}

    export(QMAKE_EXTRA_TARGETS)
    export(GENERATED_SOURCES)
    export(PRE_TARGETDEPS)
}

generateStub(Adapter)
generateStub(Daemon)
generateStub(IsoDep)
generateStub(Settings)
generateStub(Tag)

# libqnfcdc

LIBQNFCDC_INCLUDE = $${LIBQNFCDC_DIR}/include
LIBQNFCDC_SRC = $${LIBQNFCDC_DIR}/src

INCLUDEPATH += \
    $${LIBQNFCDC_INCLUDE}

HEADERS += \
    $${LIBQNFCDC_INCLUDE}/NfcAdapter.h \
    $${LIBQNFCDC_INCLUDE}/NfcMode.h \
    $${LIBQNFCDC_INCLUDE}/NfcSystem.h \
    $${LIBQNFCDC_INCLUDE}/NfcTag.h

SOURCES += \
    $${LIBQNFCDC_SRC}/NfcAdapter.cpp \
    $${LIBQNFCDC_SRC}/NfcMode.cpp \
    $${LIBQNFCDC_SRC}/NfcSystem.cpp \
    $${LIBQNFCDC_SRC}/NfcTag.cpp

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
    src/TravelCard.h \
    src/TravelCardImpl.h \
    src/Util.h

SOURCES += \
    src/main.cpp \
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
ICON_SIZES = 86 108 128 172 256
for(s, ICON_SIZES) {
    icon_target = icon_$${s}
    icon_dir = icons/$${s}x$${s}
    $${icon_target}.files = $${icon_dir}/$${TARGET}.png
    $${icon_target}.path = /usr/share/icons/hicolor/$${s}x$${s}/apps
    INSTALLS += $${icon_target}
}

# Translations
TRANSLATION_IDBASED=-idbased
TRANSLATION_SOURCES = \
  $${_PRO_FILE_PWD_}/qml

defineTest(addTrFile) {
    rel = translations/$${1}
    OTHER_FILES += $${rel}.ts
    export(OTHER_FILES)

    in = $${_PRO_FILE_PWD_}/$$rel
    out = $${OUT_PWD}/$$rel

    s = $$replace(1,-,_)
    lupdate_target = lupdate_$$s
    qm_target = qm_$$s

    $${lupdate_target}.commands = lupdate -noobsolete -locations none $${TRANSLATION_SOURCES} -ts \"$${in}.ts\" && \
        mkdir -p \"$${OUT_PWD}/translations\" &&  [ \"$${in}.ts\" != \"$${out}.ts\" ] && \
        cp -af \"$${in}.ts\" \"$${out}.ts\" || :

    $${qm_target}.path = $$TRANSLATIONS_PATH
    $${qm_target}.depends = $${lupdate_target}
    $${qm_target}.commands = lrelease $$TRANSLATION_IDBASED \"$${out}.ts\" && \
        $(INSTALL_FILE) \"$${out}.qm\" $(INSTALL_ROOT)$${TRANSLATIONS_PATH}/

    QMAKE_EXTRA_TARGETS += $${lupdate_target} $${qm_target}
    INSTALLS += $${qm_target}

    export($${lupdate_target}.commands)
    export($${qm_target}.path)
    export($${qm_target}.depends)
    export($${qm_target}.commands)
    export(QMAKE_EXTRA_TARGETS)
    export(INSTALLS)
}

LANGUAGES = fi pl ru sv zh_CN

addTrFile($${TARGET})
for(l, LANGUAGES) {
    addTrFile($${TARGET}-$$l)
}

qm.path = $$TRANSLATIONS_PATH
qm.CONFIG += no_check_exist
INSTALLS += qm

OTHER_FILES += LICENSE
