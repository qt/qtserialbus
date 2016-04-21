QT += serialbus widgets

TARGET = can
TEMPLATE = app

SOURCES += main.cpp \
    mainwindow.cpp \
    settingsdialog.cpp

HEADERS += mainwindow.h \
    settingsdialog.h

FORMS   += mainwindow.ui \
    settingsdialog.ui

RESOURCES += can.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/serialbus/can
INSTALLS += target
