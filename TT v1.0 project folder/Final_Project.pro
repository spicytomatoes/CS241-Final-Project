QT       += core gui charts concurrent location quick positioning

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# CHECK IF THIS IS THE RIGHT PATH
INCLUDEPATH += C:\Qt\Tools\OpenSSL\Win_x86\bin

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    barproxymodel.cpp \
    main.cpp \
    mainwindow.cpp \
    summedchart.cpp \
    summedtablemodel.cpp

HEADERS += \
    barproxymodel.h \
    mainwindow.h \
    summedchart.h \
    summedtablemodel.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES +=

RC_ICONS = logo.ico

RESOURCES += \
    main.qrc
