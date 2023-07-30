/*
 * Copyright (C) 2019-2023 Slava Monich <slava@monich.com>
 * Copyright (C) 2019-2022 Jolla Ltd.
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in
 *      the documentation and/or other materials provided with the
 *      distribution.
 *   3. Neither the names of the copyright holders nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the authors and should not be interpreted as representing
 * any official policies, either expressed or implied.
 */

#include "TravelCard.h"

#include "NfcAdapter.h"
#include "NfcMode.h"
#include "NfcSystem.h"

#include "HarbourDebug.h"
#include "HarbourSystemTime.h"
#include "HarbourUtil.h"

#include "gutil_log.h"

#include <sailfishapp.h>

#include <QGuiApplication>
#include <QtQuick>

#define APP_NAME  "harbour-matkakortti"
#define APP_QML_IMPORT  "harbour.matkakortti"

static void register_types(const char* uri, int v1 = 1, int v2 = 0)
{
#define REGISTER_TYPE(uri, v1, v2, Class) \
    qmlRegisterType<Class>(uri, v1, v2, #Class)
#define REGISTER_SINGLETON_TYPE(uri, v1, v2, Class) \
    qmlRegisterSingletonType<Class>(uri, v1, v2, #Class, \
    Class::createSingleton)

    REGISTER_SINGLETON_TYPE(uri, v1, v2, HarbourSystemTime);
    REGISTER_SINGLETON_TYPE(uri, v1, v2, HarbourUtil);
    REGISTER_SINGLETON_TYPE(uri, v1, v2, NfcAdapter);
    REGISTER_SINGLETON_TYPE(uri, v1, v2, NfcSystem);
    REGISTER_TYPE(uri, v1, v2, NfcMode);
    REGISTER_TYPE(uri, v1, v2, TravelCard);
    TravelCard::registerTypes(uri, v1, v2);
}

int main(int argc, char *argv[])
{
    QGuiApplication* app = SailfishApp::application(argc, argv);

    app->setApplicationName(APP_NAME);
    register_types(APP_QML_IMPORT, 1, 0);

    // Load translations
    QLocale locale;
    QTranslator* tr = new QTranslator(app);
#ifdef OPENREPOS
    // OpenRepos build has settings applet
    const QString transDir("/usr/share/translations");
#else
    const QString transDir = SailfishApp::pathTo("translations").toLocalFile();
#endif
    const QString transFile(APP_NAME);
    if (tr->load(locale, transFile, "-", transDir) ||
        tr->load(transFile, transDir)) {
        app->installTranslator(tr);
    } else {
        HDEBUG("Failed to load translator for" << locale);
        delete tr;
    }

#ifdef DEBUG
    gutil_log_default.level = GLOG_LEVEL_VERBOSE;
#endif

    // Create the view
    QQuickView* view = SailfishApp::createView();

    // Initialize the view and show it
    view->setTitle(qtTrId("matkakortti-app_name"));
    view->setSource(SailfishApp::pathTo("qml/main.qml"));
    view->showFullScreen();

    int ret = app->exec();

    delete view;
    delete app;
    return ret;
}
