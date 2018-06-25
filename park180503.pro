TEMPLATE = app
CONFIG += console
CONFIG -= qt

SOURCES += main.cpp \
    net_march.cpp \
    joint_3960.cpp \
    park_lk.cpp \
    route_dis.cpp \
    lock_ptl.cpp \
    lock_485.cpp \
    queue_com.cpp

HEADERS += \
    global.h \
    net_march.h \
    joint_3960.h \
    park_lk.h \
    route_dis.h \
    lock_ptl.h \
    lock_485.h \
    queue_com.h

