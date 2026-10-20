# core-paths.pri
# Устанавливает DESTDIR/OBJECTS_DIR/MOC_DIR/RCC_DIR/UI_DIR.
# Требует: DESTINATION_PATH (из qmake-destination-path.pri), TARGET (задан в .pro).
# Включается автоматически из core-build.pri, если не указан CONFIG += core_no_paths.

isEmpty(DESTINATION_PATH): error("core-paths.pri: DESTINATION_PATH is empty")
isEmpty(TARGET):           error("core-paths.pri: TARGET is empty")

CORE_BINARIES_DIR = $$_PRO_FILE_PWD_/../../$${TARGET}-binaries/$${DESTINATION_PATH}
CORE_BUILD_DIR    = $$_PRO_FILE_PWD_/../../$${TARGET}-build/$${DESTINATION_PATH}

DESTDIR     = $$CORE_BINARIES_DIR
OBJECTS_DIR = $$CORE_BUILD_DIR/.obj
MOC_DIR     = $$CORE_BUILD_DIR/.moc
RCC_DIR     = $$CORE_BUILD_DIR/.rcc
UI_DIR      = $$CORE_BUILD_DIR/.ui