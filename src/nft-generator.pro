#==========================================
# NFT GENERATOR
#==========================================

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++11
#CONFIG += console
CODECFORSRC = UTF-8

# GENERAL
TARGET = nft_generator
TEMPLATE = app
VERSION = 1.0.0

# DIRS
CONFIG -= debug_and_release debug_and_release_target
DESTDIR = $$PWD/../builddir
OBJECTS_DIR = $$DESTDIR/obj
MOC_DIR = $$DESTDIR/moc
UI_DIR = $$DESTDIR/ui
RCC_DIR = $$DESTDIR/rcc

#DEFINES
#DEFINES += QT_DEPRECATED_X

#FILES
SOURCES += \
    dialog.cpp \
    generator.cpp \
    main.cpp \
    mainwindow.cpp \
    message.cpp \
    progressmessage.cpp

HEADERS += \
    constants.h \
    dialog.h \
    generator.h \
    mainwindow.h \
    message.h \
    progressmessage.h

FORMS += \
    dialog.ui \
    mainwindow.ui \
    message.ui \
    progressmessage.ui

# INSTALLS
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    rcc.qrc
