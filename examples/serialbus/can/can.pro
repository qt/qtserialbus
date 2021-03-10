QT += serialbus widgets
requires(qtConfig(combobox))

TARGET = can
TEMPLATE = app

SOURCES += \
    bitratebox.cpp \
    connectdialog.cpp \
    main.cpp \
    mainwindow.cpp \
    receivedframesmodel.cpp \
    receivedframesview.cpp \
    sendframebox.cpp

HEADERS += \
    bitratebox.h \
    common.h \
    connectdialog.h \
    mainwindow.h \
    receivedframesmodel.h \
    receivedframesview.h \
    sendframebox.h

FORMS   += mainwindow.ui \
    connectdialog.ui \
    sendframebox.ui

RESOURCES += can.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/serialbus/can
INSTALLS += target
