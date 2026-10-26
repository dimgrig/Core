# qmake-version.pri — единый источник версии Core.
# Читает VERSION из корня Core

isEmpty(CORE_ROOT): CORE_ROOT = $$PWD

_CORE_VERSION_FILE = $$CORE_ROOT/VERSION
!exists($$_CORE_VERSION_FILE): error("VERSION file not found: $$_CORE_VERSION_FILE")

win32 {
    CORE_VERSION = $$system(powershell -NoProfile -Command "Get-Content -Raw '$$_CORE_VERSION_FILE'")
}
unix {
    CORE_VERSION = $$system(cat $$_CORE_VERSION_FILE)
}

# Убираем CR/LF/пробелы (Get-Content -Raw возвращает с \r\n)
CORE_VERSION = $$replace(CORE_VERSION, "\\s", "")

isEmpty(CORE_VERSION): error("Cannot read CORE_VERSION from $$_CORE_VERSION_FILE")

CORE_VERSION_MAJOR = $$section(CORE_VERSION, ., 0, 0)
isEmpty(CORE_VERSION_MAJOR): error("Cannot parse major from CORE_VERSION='$$CORE_VERSION'")

message("CORE_VERSION = $$CORE_VERSION (major $$CORE_VERSION_MAJOR)")
