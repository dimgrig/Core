TEMPLATE = subdirs
SUBDIRS = core-lib \
        core-io-lib \
        core-journal-lib \
        core-modbus-lib \
        core-mvc-lib \
        core-utils-lib \
        core-widgets-lib \
        muParser

core-io-lib.depends = core-lib core-modbus-lib
core-journal-lib.depends = core-lib
core-mvc-lib.depends = core-lib core-utils-lib core-widgets-lib
core-modbus-lib.depends = core-lib
core-utils-lib.depends = core-lib core-widgets-lib muParser
core-widgets-lib.depends = core-lib

CONFIG(debug, debug|release) {
    SUBDIRS += tests

    tests.depends = core-lib \
                    core-io-lib \
                    core-journal-lib \
                    core-modbus-lib \
                    core-mvc-lib \
                    core-utils-lib \
                    core-widgets-lib \
                    muParser
}
