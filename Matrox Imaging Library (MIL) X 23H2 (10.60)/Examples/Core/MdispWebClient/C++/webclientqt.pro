lessThan(QT_MAJOR_VERSION, 5): error("requires Qt 5")
TARGET = MdispWebClient
TEMPLATE = app
CONFIG += qt
QT += widgets core gui 
unix:INCLUDEPATH += ${MILDIR}/include
win32:INCLUDEPATH += $$quote($$(mil_path))/../Include
unix:LIBS += -L${MILDIR}/lib -lmilwebclient
win32:LIBS += $$quote($$(mil_path))/../lib/milwebclient.lib
SOURCES += webclient.cpp webclientqt.cpp
HEADERS += webclient.h webclientqt.h
