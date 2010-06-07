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

message($$QMAKE_HOST.arch)
contains(QMAKE_HOST.arch, "x86_64") {
  message(Building 64-bit)
  QMAKE_LIBDIR += /usr/lib64/mysql
}

LIBS += -lkdeui \
    -lnepomuk \
    -lkexiv2 \
    -ltag \
    -L/usr/lib/mysql \
    -lmysqld \
    -lcrypt \
    -lssl

HEADERS += AmarokCollection.h \
    ID3Utilities.h

