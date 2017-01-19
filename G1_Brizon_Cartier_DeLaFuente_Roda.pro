TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_MAC_SDK = macosx10.12

unix:!macx {
    LIBS += -static
}


SOURCES += main.cpp \

HEADERS += \
