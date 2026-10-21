# core-build.pri — единая точка входа. Include в конец каждого .pro.
# Перед include при необходимости задать:
#   CORE_LINK_LIBS       — список core-библиотек (core-lib core-journal-lib ...)
#   CORE_DEPLOY_PREFIX   — префикс для deploy (по умолчанию lib)
#   CORE_DEPLOY_TARGET   — имя (по умолчанию $$TARGET)
#   CONFIG += core_no_deploy           — не деплоить
#   CONFIG += core_no_paths            — не переопределять DESTDIR/OBJECTS_DIR
#   CONFIG += core_no_default_defines  — не добавлять NOCRYPT NOUSER и -Wno-deprecated

isEmpty(CORE_ROOT): CORE_ROOT = $$PWD
isEmpty(LINUX_LIBS_PATH): LINUX_LIBS_PATH = /usr/lib/qt/
isEmpty(WIN_LIBS_PATH):   WIN_LIBS_PATH   = C:/QtCreatorProjects/libs
isEmpty(WIN_LXERCES_PATH): WIN_LXERCES_PATH = $$WIN_LIBS_PATH

# Платформа/версия/пути
include($$CORE_ROOT/qmake-target-platform.pri)
include($$CORE_ROOT/qmake-core-version.pri)
include($$CORE_ROOT/qmake-destination-path.pri)

# Общие defines
!contains(CONFIG, core_no_default_defines) {
    DEFINES += NOCRYPT NOUSER
    QMAKE_CXXFLAGS += -Wno-deprecated
}

# Astra / GCC10
include($$CORE_ROOT/core-astra.pri)

# INCLUDEPATH для sibling-библиотек и сторонних
for(_core_lib, CORE_LINK_LIBS) {
    _dir = $$CORE_ROOT/$$_core_lib
    exists($$_dir): INCLUDEPATH += $$_dir
}
contains(CORE_LINK_LIBS, muParser) {
    INCLUDEPATH += $$CORE_ROOT/muParser/include
    DEFINES += MUPARSER_STATIC
}
contains(CORE_LINK_LIBS, qcustomplot) {
    QMAKE_QCUSTOMPLOT_VERSION_OVERRIDE = 2
    INCLUDEPATH += $$CORE_ROOT/qcustomplot
}

# Build paths
!contains(CONFIG, core_no_paths) {
    include($$CORE_ROOT/core-paths.pri)
}

# Deploy
!contains(CONFIG, core_no_deploy) {
    include($$CORE_ROOT/core-deploy.pri)
}

# LIBS для зависимых core-библиотек
!isEmpty(CORE_LINK_LIBS) {
    include($$CORE_ROOT/core-link.pri)
}
