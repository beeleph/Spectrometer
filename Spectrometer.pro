QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    WDconfig.c \
    WDplot.c \
    WaveDump.c \
    flash.c \
    keyb.c \
    main.cpp \
    mainwindow.cpp \
    spi.c

HEADERS += \
    CAENComm.h \
    CAENDigitizer.h \
    CAENDigitizerType.h \
    WDconfig.h \
    WDplot.h \
    WaveDump.h \
    flash.h \
    flash_opcodes.h \
    keyb.h \
    mainwindow.h \
    spi.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    README.md \
    cfg/WaveDumpConfig.txt \
    cfg/WaveDumpConfig_X740.txt \
    cfg/WaveDumpConfig_X742.txt \
    dll/x32/CAENComm.dll \
    dll/x32/CAENDigitizer.dll \
    dll/x32/CAENVMELib.dll \
    dll/x32/PlxApi.dll \
    dll/x64/CAENComm.dll \
    dll/x64/CAENDigitizer.dll \
    dll/x64/CAENVMELib.dll \
    dll/x64/PlxApi.dll \
    lib/Win32/Debug/CAENComm.lib \
    lib/Win32/Debug/CAENDigitizer.lib \
    lib/Win32/Release/CAENComm.lib \
    lib/Win32/Release/CAENDigitizer.lib \
    lib/x64/Debug/CAENComm.lib \
    lib/x64/Debug/CAENDigitizer.lib \
    lib/x64/Release/CAENComm.lib \
    lib/x64/Release/CAENDigitizer.lib

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/lib/Win32/release/ -lCAENComm
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/lib/Win32/debug/ -lCAENComm

INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/lib/Win32/release/ -lCAENDigitizer
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/lib/Win32/debug/ -lCAENDigitizer

INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.
