#-------------------------------------------------
#
# Project created by QtCreator 2011-03-09T20:51:16
#
#-------------------------------------------------

CONFIG += link_pkgconfig
PKGCONFIG += libpulse libpulse-mainloop-glib

QT       += core gui network
!mac:LIBS += -ldns_sd
TARGET = meego-continuum 
TEMPLATE = app


SOURCES += main.cpp\
           mainwindow.cpp \
           pulseasync.cpp

HEADERS  += mainwindow.h \
            pulseasync.h

FORMS    += mainwindow.ui
