lessThan(QT_MAJOR_VERSION, 5): error("requires Qt 5")
TARGET = MdispWindowQt
TEMPLATE = app
CONFIG += qt
QT += widgets core gui 
unix:INCLUDEPATH += $$(MILDIR)/include
win32:INCLUDEPATH += $$quote($$(mil_path))/../Include
unix:LIBS += -L$$(MILDIR)/lib -lmil -lX11
win32:LIBS += $$quote($$(mil_path))/../lib/mil.lib
SOURCES += MdispWindowQt.cpp
HEADERS += MdispWindowQt.h
