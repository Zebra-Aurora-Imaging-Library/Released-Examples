lessThan(QT_MAJOR_VERSION, 5): error("requires Qt 5")
TARGET = MdispQt
TEMPLATE = app
unix:CONFIG+=warn_off link_pkgconfig
unix:CONFIG += warn_on
unix:QT += widgets core gui x11extras
win32:QT += widgets core gui
unix:INCLUDEPATH += $$(MILDIR)/include
win32:INCLUDEPATH += $$quote($$(mil_path))/../Include
unix:LIBS += -L$$(MILDIR)/lib -lmil -lX11
win32:LIBS += $$quote($$(mil_path))/../lib/mil.lib  gdi32.lib User32.lib
unix:PKGCONFIG+=cairo-xlib
SOURCES += MdispQt.cpp \
    mainframe.cpp \
    mdispqtapp.cpp \
    aboutbox.cpp \
    childframe.cpp \
    mdispqtview.cpp
HEADERS += mainframe.h \
    mdispqtapp.h \
    aboutbox.h \
    childframe.h \
    mdispqtview.h
FORMS += mainframe.ui \
    aboutbox.ui
RESOURCES += mdispqt.qrc
