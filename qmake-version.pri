# qmake-version.pri — единый источник версии Core.
# Читает VERSION из корня Core. Все whitespace удаляются на уровне шелла —
# надёжнее, чем $$split()/$$replace() в qmake.

isEmpty(CORE_ROOT): CORE_ROOT = $$PWD

_CORE_VERSION_FILE = $$CORE_ROOT/VERSION
!exists($$_CORE_VERSION_FILE): error("VERSION file not found: $$_CORE_VERSION_FILE")

# tr -d "[:space:]" убирает пробелы, табы, CR, LF — включая trailing space и Windows CRLF
unix {
    CORE_VERSION = $$system(tr -d "[:space:]" < $$shell_path($$_CORE_VERSION_FILE))
}
win32 {
    CORE_VERSION = $$system(tr -d "[:space:]" < $$shell_path($$_CORE_VERSION_FILE))
    isEmpty(CORE_VERSION) {
        # fallback: если tr нет в PATH (редкий MinGW), пробуем sed
        CORE_VERSION = $$system(sed -e "s/[[:space:]]//g" $$shell_path($$_CORE_VERSION_FILE))
    }
}

isEmpty(CORE_VERSION) {
    # Диагностика — попадёт в Compile Output Qt Creator
    _CORE_VERSION_RAW = $$system(cat $$shell_path($$_CORE_VERSION_FILE))
    message("DEBUG: file = $$_CORE_VERSION_FILE")
    message("DEBUG: exists = $$exists($$_CORE_VERSION_FILE)")
    message("DEBUG: raw cat = '$$_CORE_VERSION_RAW'")
    message("DEBUG: shell echo = '$$system(echo test)'")
    error("VERSION file is empty or unreadable: $$_CORE_VERSION_FILE")
}

CORE_VERSION_MAJOR = $$section(CORE_VERSION, ., 0, 0)
isEmpty(CORE_VERSION_MAJOR): error("Cannot parse major from CORE_VERSION='$$CORE_VERSION'")

message("CORE_VERSION = $$CORE_VERSION (major $$CORE_VERSION_MAJOR)")
