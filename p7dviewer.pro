QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = p7dviewer
TEMPLATE = app

greaterThan(QT_MINOR_VERSION, 4) { # >= 5.5
    CONFIG  += c++14
} else {
    CONFIG  += c++11
}

# below doesn't work on win with old Qt
unix:CONFIG += object_parallel_to_source
unix:OBJECTS_DIR = .

QMAKE_LFLAGS += -Wl,--no-as-needed, -fexceptions# ?

SOURCES  += main.cpp \
            main_window.cpp \
            p7d_model.cpp


HEADERS  += Formatter.h \
            GTypes.h \
            p7Structs.h \
            importer.h \
            main_window.h \
            p7d_model.h

