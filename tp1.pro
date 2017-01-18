TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_MAC_SDK = macosx10.12

LIBS +=   -static

#-l"$$_PRO_FILE_PWD_/sound"/libportaudio.a

SOURCES += main.cpp \
   # sound.cpp

HEADERS += \
  #  sound/portaudio.h \
  #  sound.h
