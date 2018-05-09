TEMPLATE = app
CONFIG += c++11
INCLUDEPATH += .
TARGET = adueditor
QT += serialbus serialport widgets
QT += serialbus-private core-private
requires(qtConfig(combobox))
requires(qtConfig(modbus-serialport))

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    modbustcpclient.cpp

HEADERS += \
    mainwindow.h  \
    modbustcpclient.h \
    modbustcpclient_p.h \
    plaintextedit.h

FORMS += \
    interface.ui

target.path = $$[QT_INSTALL_EXAMPLES]/serialbus/modbus/adueditor
INSTALLS += target
