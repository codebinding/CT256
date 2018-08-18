TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

LIBS += \
        -pthread

SOURCES += \
        main.cpp \
    fpgabridge.cpp \
    canpacket.cpp \
    socketcan.cpp

HEADERS += \
    register.h \
    fpgabridge.h \
    exception.h \
    canpacket.h \
    socketcan.h
