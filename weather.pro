
include(../dde-dock/interfaces/interfaces.pri)

QT              += widgets svg network
TEMPLATE         = lib
CONFIG          += plugin c++11 link_pkgconfig
PKGCONFIG       += dtkwidget

#TARGET          = $$qtLibraryTarget(datetime)
TARGET           = Weather
#DESTDIR          = $$_PRO_FILE_PWD_/../
DISTFILES       += weather.json

HEADERS += \
    weatherplugin.h \
    weatherwidget.h

SOURCES += \
    weatherplugin.cpp \
    weatherwidget.cpp

target.path = $${PREFIX}/lib/dde-dock/plugins/
INSTALLS += target

RESOURCES += res.qrc
