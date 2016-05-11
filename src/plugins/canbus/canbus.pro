TEMPLATE = subdirs

config_socketcan {
    SUBDIRS += socketcan
}

SUBDIRS += peakcan tinycan
win32:SUBDIRS += vectorcan
