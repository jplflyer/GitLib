TEMPLATE = app

CONFIG += console
CONFIG += c++17
CONFIG += sdk_no_version_check

CONFIG -= app_bundle
CONFIG -= qt

QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15

INCLUDEPATH += /usr/local/include

LIBS += -L/usr/local/opt/openssl/lib -L/usr/local/lib
LIBS += -lshow-mac
LIBS += -framework CoreFoundation
LIBS +=  -lssl -lcrypto

SOURCES += \
	programs/GitFetch.cpp \
	programs/GitInfo.cpp \
	src/Git.cpp

HEADERS += \
	src/Git.h
