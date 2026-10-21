# qmake-core-version.pri — Core install paths (аналог cmake/Paths.cmake).
# Определяет CORE_INSTALL_PREFIX, CORE_INSTALL_{LIB,INCLUDE,BIN}DIR,
# CORE_APP_LOCAL_DIR (win staging) и legacy-алиасы LINUX_LIBS_PATH,
# WIN_LIBS_PATH (нужны core-link.pri / core-deploy.pri).
#
# Требует: CORE_VERSION (из qmake-version.pri).

isEmpty(CORE_VERSION): error("qmake-core-version.pri: CORE_VERSION is empty. Include qmake-version.pri first.")

# ── Priority 1: env CORE_INSTALL_PREFIX (Docker/CI/staging) ─
_CORE_INSTALL_PREFIX_ENV = $$(CORE_INSTALL_PREFIX)
!isEmpty(_CORE_INSTALL_PREFIX_ENV): CORE_INSTALL_PREFIX = $$_CORE_INSTALL_PREFIX_ENV

# ── Priority 2: дефолт по платформе ─────────────────────────
isEmpty(CORE_INSTALL_PREFIX) {
    win32|win64 {
        _LOCALAPPDATA = $$(LOCALAPPDATA)
        !isEmpty(_LOCALAPPDATA): CORE_INSTALL_PREFIX = $$shell_path($$LOCALAPPDATA/Core/$$CORE_VERSION)
        else: CORE_INSTALL_PREFIX = $$shell_path($$PWD/../../applocal-staging)
    } else {
        # Нативные qmake-сборки на хосте: user-writable каталог (без sudo).
        # В Docker/CI — env CORE_INSTALL_PREFIX=/opt/core/$$CORE_VERSION перебивает.
        _HOME = $$(HOME)
        !isEmpty(_HOME): CORE_INSTALL_PREFIX = $$_HOME/.local/core/$$CORE_VERSION
        else:             CORE_INSTALL_PREFIX = /opt/core/$$CORE_VERSION
    }
}

# ── Install subdirs (relative to prefix — как в CMake) ──────
isEmpty(CORE_INSTALL_INCLUDEDIR): CORE_INSTALL_INCLUDEDIR = include
isEmpty(CORE_INSTALL_LIBDIR):     CORE_INSTALL_LIBDIR     = lib
isEmpty(CORE_INSTALL_BINDIR):     CORE_INSTALL_BINDIR     = bin
isEmpty(CORE_INSTALL_CMAKEDIR):   CORE_INSTALL_CMAKEDIR   = lib/cmake/Core

# ── Windows applocal staging (для кросс-сборки из Docker) ───
_CORE_APP_LOCAL_ENV = $$(CORE_APP_LOCAL_DIR)
!isEmpty(_CORE_APP_LOCAL_ENV): CORE_APP_LOCAL_DIR = $$_CORE_APP_LOCAL_ENV
isEmpty(CORE_APP_LOCAL_DIR): CORE_APP_LOCAL_DIR = $$CORE_INSTALL_PREFIX

# ── Legacy aliases (используются core-link.pri / core-deploy.pri) ──
LINUX_LIBS_PATH = $$CORE_INSTALL_PREFIX/$$CORE_INSTALL_LIBDIR
WIN_LIBS_PATH   = $$CORE_APP_LOCAL_DIR/$$CORE_INSTALL_LIBDIR
isEmpty(WIN_LXERCES_PATH): WIN_LXERCES_PATH = $$WIN_LIBS_PATH

# ── Env-override на legacy-пути (если кто-то держит свои либы в старом месте) ──
_CORE_LINUX_LIBS_PATH_ENV = $$(CORE_LINUX_LIBS_PATH)
!isEmpty(_CORE_LINUX_LIBS_PATH_ENV): LINUX_LIBS_PATH = $$_CORE_LINUX_LIBS_PATH_ENV

_CORE_WIN_LIBS_PATH_ENV = $$(CORE_WIN_LIBS_PATH)
!isEmpty(_CORE_WIN_LIBS_PATH_ENV):   WIN_LIBS_PATH   = $$_CORE_WIN_LIBS_PATH_ENV

_CORE_WIN_LXERCES_ENV = $$(CORE_WIN_LXERCES_PATH)
!isEmpty(_CORE_WIN_LXERCES_ENV):     WIN_LXERCES_PATH = $$_CORE_WIN_LXERCES_ENV

message("CORE_INSTALL_PREFIX = $$CORE_INSTALL_PREFIX")
message("LINUX_LIBS_PATH     = $$LINUX_LIBS_PATH")
message("WIN_LIBS_PATH       = $$WIN_LIBS_PATH")
