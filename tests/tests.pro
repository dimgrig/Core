QT += gui core widgets network serialport serialbus mqtt printsupport sql testlib # подключаем QtTest

TEMPLATE = app
TARGET = core-tests
CONFIG += c++17
CONFIG += core_no_deploy    # tests — приложение, deploy-копирование .so/.dll не нужно

INCLUDEPATH += "$$_PRO_FILE_PWD_/"

CORE_LINK_LIBS = core-lib core-journal-lib core-modbus-lib \
                 core-io-lib core-mvc-lib core-utils-lib core-widgets-lib \
                 muParser
include(../core-build.pri)

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
