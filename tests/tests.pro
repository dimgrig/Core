QT += gui core widgets network serialport serialbus mqtt printsupport sql testlib # подключаем QtTest

TEMPLATE = app
TARGET = core-tests    # имя исполняемого файла тестов
CONFIG += c++17

include(../qmake-target-platform.pri)   # если эти файлы лежат в корне
include(../qmake-core-version.pri)
include(../qmake-destination-path.pri)

exists(/etc/astra_version) {
    LIBS += -L$${LINUX_LIBS_PATH} -lcore-lib
    LIBS += -L$${LINUX_LIBS_PATH} -lcore-io-lib
    LIBS += -L$${LINUX_LIBS_PATH} -lcore-journal-lib
    LIBS += -L$${LINUX_LIBS_PATH} -lcore-modbus-lib
    LIBS += -L$${LINUX_LIBS_PATH} -lcore-mvc-lib
    LIBS += -L$${LINUX_LIBS_PATH} -lcore-utils-lib
    LIBS += -L$${LINUX_LIBS_PATH} -lcore-widgets-lib
    LIBS += -L/usr/local/lib/ -lmuparser

    DISTFILES += core-gcc10.pri
    include(core-gcc10.pri)
} else {
    unix {
        LIBS += -ldl
        LIBS += -L$${LINUX_LIBS_PATH} -lcore-lib
        LIBS += -L$${LINUX_LIBS_PATH} -lcore-io-lib
        LIBS += -L$${LINUX_LIBS_PATH} -lcore-journal-lib
        LIBS += -L$${LINUX_LIBS_PATH} -lcore-modbus-lib
        LIBS += -L$${LINUX_LIBS_PATH} -lcore-mvc-lib
        LIBS += -L$${LINUX_LIBS_PATH} -lcore-utils-lib
        LIBS += -L$${LINUX_LIBS_PATH} -lcore-widgets-lib
        LIBS += -L$${LINUX_LIBS_PATH} -lmuParser
    }

    win32 {
        DEFINES += win32
        LIBS += -L$${WIN_LIBS_PATH} -lcore-lib
        LIBS += -L$${WIN_LIBS_PATH} -lcore-io-lib
        LIBS += -L$${WIN_LIBS_PATH} -lcore-journal-lib
        LIBS += -L$${WIN_LIBS_PATH} -lcore-modbus-lib
        LIBS += -L$${WIN_LIBS_PATH} -lcore-mvc-lib
        LIBS += -L$${WIN_LIBS_PATH} -lcore-utils-lib
        LIBS += -L$${WIN_LIBS_PATH} -lcore-widgets-lib
        LIBS += -L$${WIN_LIBS_PATH} -lmuParser
    }
    win64 {
        DEFINES += win64
        LIBS += -L$${WIN_LIBS_PATH} -lcore-lib
        LIBS += -L$${WIN_LIBS_PATH} -lcore-io-lib
        LIBS += -L$${WIN_LIBS_PATH} -lcore-journal-lib
        LIBS += -L$${WIN_LIBS_PATH} -lcore-modbus-lib
        LIBS += -L$${WIN_LIBS_PATH} -lcore-mvc-lib
        LIBS += -L$${WIN_LIBS_PATH} -lcore-utils-lib
        LIBS += -L$${WIN_LIBS_PATH} -lcore-widgets-lib
        LIBS += -L$${WIN_LIBS_PATH} -lmuParser
    }
}

# Пути к заголовкам библиотеки
# чтобы видеть заголовки библиотеки (они лежат на уровень выше)
INCLUDEPATH += "$$_PRO_FILE_PWD_/"

INCLUDEPATH += "$$_PRO_FILE_PWD_/../core-lib/"
INCLUDEPATH += "$$_PRO_FILE_PWD_/../core-io-lib/"
INCLUDEPATH += "$$_PRO_FILE_PWD_/../core-journal-lib/"
INCLUDEPATH += "$$_PRO_FILE_PWD_/../core-modbus-lib/"
INCLUDEPATH += "$$_PRO_FILE_PWD_/../core-modbus-lib/bindings"
INCLUDEPATH += "$$_PRO_FILE_PWD_/../core-mvc-lib/"
INCLUDEPATH += "$$_PRO_FILE_PWD_/../core-utils-lib/"
INCLUDEPATH += "$$_PRO_FILE_PWD_/../core-widgets-lib/"
INCLUDEPATH += "$$_PRO_FILE_PWD_/../muParser/include"

BUILD_DIR = $$PWD/../../build/$${TARGET}-build/
DESTDIR = $$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH
OBJECTS_DIR = $$PWD/../../$${TARGET}-build/$$DESTINATION_PATH/.obj
MOC_DIR = $$PWD/../../../$${TARGET}-build/$$DESTINATION_PATH/.moc
RCC_DIR = $$PWD/../../../$${TARGET}-build/$$DESTINATION_PATH/.qrc
UI_DIR = $$PWD/../../../$${TARGET}-build/$$DESTINATION_PATH/.ui

# Запуск тестов сразу после сборки (кросс-платформенно)
exists(/etc/astra_version) {

} else {
    unix {
        QMAKE_POST_LINK = LD_LIBRARY_PATH=$$shell_path($${LINUX_LIBS_PATH}):$$LD_LIBRARY_PATH $$shell_path($$DESTDIR/$$TARGET)
    }
    #    win32|win64 {
    #        #QMAKE_POST_LINK = set PATH=$$shell_path($${WIN_LIBS_PATH});%PATH% && $$shell_path($$DESTDIR/$$TARGET.exe)
    #        QMAKE_POST_LINK = cmd /c "set PATH=$${WIN_LIBS_PATH};%PATH% && $$DESTDIR/$${TARGET}.exe"
    #    }
}

#DIR_ROOT = $$PWD/../

#QMAKE_LFLAGS += -Wl,--allow-multiple-definition
#{
#    # Добавляем все подпапки, где лежат исходники (перечислите все каталоги, которые есть в проекте)
#    TESTS_SUBDIRS = core-lib \
#                    core-io-lib \
#                    core-journal-lib \
#                    core-modbus-lib \
#                    core-mvc-lib \
#                    core-utils-lib \
#                    core-utils-lib\muParser \
#                    core-widgets-lib

#    # Собираем все .cpp файлы из этих папок, кроме main.cpp
#    for(dir, TESTS_SUBDIRS) {
#        #message($$dir)
#        FILES = $$files($$DIR_ROOT/$$dir/*.cpp)
#        for(file, FILES) {
#            # Исключаем main.cpp, если он там есть (обычно main.cpp лежит в корне)
#            name = $$basename(file)
#            !equals(name, main.cpp) {
#                SOURCES += $$file
#            }
#        }

#        UI_FILES = $$files($$DIR_ROOT/$$dir/uis/*.ui)
#        for(file, UI_FILES) {
#            #message($$file)
#            FORMS += $$file
#        }
#    }

#    UI_FILES = $$files($$DIR_ROOT/uis/*.ui)
#    for(file, UI_FILES) {
#        FORMS += $$file
#    }

#    # Также добавить .cpp из корня, исключая main.cpp
#    ROOT_FILES = $$files($$DIR_ROOT/*.cpp)
#    for(file, ROOT_FILES) {
#        name = $$basename(file)
#        !equals(name, main.cpp) {
#            SOURCES += $$file
#        }
#    }
#}

HEADERS += \
    TestAuth.h \
    TestJournal.h \
    TestModbusIntegration.h

SOURCES += \
    TestAuth.cpp \
    TestJournal.cpp \
    TestModbusIntegration.cpp \
    main.cpp \

RESOURCES += \
   resources/resources.qrc

