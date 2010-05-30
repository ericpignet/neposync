# -------------------------------------------------
# Project created by QtCreator 2010-04-26T14:24:51
# -------------------------------------------------
QT -= gui
TARGET = neposync
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app
SOURCES += main.cpp \
    AmarokCollection.cpp \
    ID3Utilities.cpp
LIBS += -lkdeui \
    -lnepomuk \
    -lkexiv2 \
    -ltag \
    -L/usr/lib/mysql \
    -lmysqld \
    -lcrypt

# OBJECTS += /usr/lib/mysql/libmysqld.a
# QT += sql
HEADERS += AmarokCollection.h \
    ID3Utilities.h
