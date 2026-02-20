TEMPLATE = subdirs
SUBDIRS = core-lib \
        core-io-lib \
        core-modbus-lib \
        core-mvc-lib \
        core-utils-lib \
        core-widgets-lib \
        tests

core-io-lib.depends = core-lib
core-mvc-lib.depends = core-lib core-utils-lib core-widgets-lib
core-modbus-lib.depends = core-lib
core-utils-lib.depends = core-lib core-widgets-lib
core-widgets-lib.depends = core-lib

tests.depends = core-lib \
                core-io-lib \
                core-modbus-lib \
                core-mvc-lib \
                core-utils-lib \
                core-widgets-lib

