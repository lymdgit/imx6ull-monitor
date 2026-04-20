QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    main.cpp \
    showphoto.cpp \
    v4l2.cpp \
    videothread.cpp

HEADERS += \
    showphoto.h \
    v4l2.h \
    videothread.h

FORMS += \
    showphoto.ui \
    v4l2.ui

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
