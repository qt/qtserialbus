%modules = (
    "QtSerialBus" => "$basedir/src/serialbus",
);

%classnames = (
    "QtSerialBus/qmodbusrtuserialserver.h" => "QModbusRtuSerialSlave",
    "QtSerialBus/qmodbusrtuserialclient.h" => "QModbusRtuSerialMaster"
),

%deprecatedheaders = (
    "QtSerialBus" => {
        "qserialbusglobal.h" => "QtSerialBus/qtserialbusglobal.h"
    },
);
