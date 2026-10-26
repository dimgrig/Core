# core-link.pri
# Добавляет LIBS для core-библиотек.
# Читает: CORE_LINK_LIBS (список имён, напр. "core-lib core-journal-lib").
# Требует: LINUX_LIBS_PATH, WIN_LIBS_PATH.

isEmpty(CORE_LINK_LIBS): CORE_LINK_LIBS = core-lib

# Библиотеки, у которых в .pro указан VERSION и которые из-за этого
# на Windows/MinGW получают версионный суффикс в имени файла:
#   qcustomplot.pro: VERSION = 2.1.1  =>  qcustomplot2.dll + libqcustomplot2.a
# На Unix версия разрешается через симлинки, поэтому суффикс нужен только для win32/win64.
#CORE_LINK_WIN_VERSIONS = qcustomplot=2 QtWebApp=1

# без этого линкер ищет libcore-widgets-lib.a, а файл называется libcore-widgets-lib1.a
#CORE_VERSION_MAJOR_V = $$section(CORE_VERSION, ., 0, 0)
#!isEmpty(CORE_VERSION_MAJOR_V) {
#    CORE_LINK_WIN_VERSIONS += \
#        core-lib=$$CORE_VERSION_MAJOR_V          \
#        core-journal-lib=$$CORE_VERSION_MAJOR_V  \
#        core-widgets-lib=$$CORE_VERSION_MAJOR_V  \
#        core-modbus-lib=$$CORE_VERSION_MAJOR_V   \
#        core-io-lib=$$CORE_VERSION_MAJOR_V       \
#        core-utils-lib=$$CORE_VERSION_MAJOR_V    \
#        core-plot-lib=$$CORE_VERSION_MAJOR_V     \
#        core-mvc-lib=$$CORE_VERSION_MAJOR_V
#}

# Все библиотеки (включая qcustomplot и QtWebApp) используют
# skip_target_version_ext из core-build.pri — на Windows собираются
# без мажорного суффикса. Поэтому список пуст.
CORE_LINK_WIN_VERSIONS =

for(_core_lib, CORE_LINK_LIBS) {
    _suffix = ""
    for(_pair, CORE_LINK_WIN_VERSIONS) {
        _kv       = $$split(_pair, "=")
        _lib_name = $$first(_kv)
        _lib_ver  = $$last(_kv)
        equals(_lib_name, $$_core_lib): _suffix = $$_lib_ver
    }
    unix:  LIBS += -L$${LINUX_LIBS_PATH} -l$${_core_lib}
    win32: LIBS += -L$${WIN_LIBS_PATH}   -l$${_core_lib}$${_suffix}
    win64: LIBS += -L$${WIN_LIBS_PATH}   -l$${_core_lib}$${_suffix}
}

