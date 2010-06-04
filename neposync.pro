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

# Could not find a way to retrieve common libdir to add the right /usr/lib/mysql dir
LIBS += -lkdeui \
    -lnepomuk \
    -lkexiv2 \
    -ltag \
    -L/usr/lib/mysql \
    -L/usr/lib64/mysql \
    -lmysqld \
    -lcrypt

HEADERS += AmarokCollection.h \
    ID3Utilities.h
