# qmake-core-version.pri — Core install paths.
# Требует: CORE_VERSION (из qmake-version.pri).

isEmpty(CORE_VERSION): error("qmake-core-version.pri: CORE_VERSION is empty.")

# ── Priority 1: env CORE_INSTALL_PREFIX ────────────────────
_CORE_INSTALL_PREFIX_ENV = $$(CORE_INSTALL_PREFIX)
!isEmpty(_CORE_INSTALL_PREFIX_ENV) {
    CORE_INSTALL_PREFIX = $$_CORE_INSTALL_PREFIX_ENV
}

isEmpty(QT_MAJOR_VERSION): QT_MAJOR_VERSION = 5
CORE_INSTALL_QT_SUBDIR = qt$${QT_MAJOR_VERSION}

# ── Priority 2: дефолт по платформе ────────────────────────
isEmpty(CORE_INSTALL_PREFIX) {
    win32|win64 {
        _LA = $$(LOCALAPPDATA)
        !isEmpty(_LA) {
            CORE_INSTALL_PREFIX = $$_LA/Core/$$CORE_INSTALL_QT_SUBDIR/$$CORE_VERSION
        } else {
            _UP = $$(USERPROFILE)
            !isEmpty(_UP) {
                CORE_INSTALL_PREFIX = $$_UP/AppData/Local/Core/$$CORE_VERSION
            } else {
                CORE_INSTALL_PREFIX = $$PWD/../../applocal-staging
            }
        }
    } else {
        _HOME = $$(HOME)
        !isEmpty(_HOME): CORE_INSTALL_PREFIX = $$_HOME/.local/core/$$CORE_INSTALL_QT_SUBDIR/$$CORE_VERSION
        else:            CORE_INSTALL_PREFIX = /opt/core/$$CORE_VERSION
    }
}

# ── Install subdirs ────────────────────────────────────────
isEmpty(CORE_INSTALL_INCLUDEDIR): CORE_INSTALL_INCLUDEDIR = include
isEmpty(CORE_INSTALL_LIBDIR):     CORE_INSTALL_LIBDIR     = lib
isEmpty(CORE_INSTALL_BINDIR):     CORE_INSTALL_BINDIR     = bin
isEmpty(CORE_INSTALL_CMAKEDIR):   CORE_INSTALL_CMAKEDIR   = lib/cmake/Core

# ── Windows applocal staging ───────────────────────────────
_CORE_APP_LOCAL_ENV = $$(CORE_APP_LOCAL_DIR)
!isEmpty(_CORE_APP_LOCAL_ENV): CORE_APP_LOCAL_DIR = $$_CORE_APP_LOCAL_ENV
isEmpty(CORE_APP_LOCAL_DIR): CORE_APP_LOCAL_DIR = $$CORE_INSTALL_PREFIX

# ── Legacy aliases ─────────────────────────────────────────
LINUX_LIBS_PATH = $$CORE_INSTALL_PREFIX/$$CORE_INSTALL_LIBDIR
WIN_LIBS_PATH   = $$CORE_APP_LOCAL_DIR/$$CORE_INSTALL_LIBDIR
isEmpty(WIN_LXERCES_PATH): WIN_LXERCES_PATH = $$WIN_LIBS_PATH

# ── Env-override ───────────────────────────────────────────
_CORE_LINUX_LIBS_PATH_ENV = $$(CORE_LINUX_LIBS_PATH)
!isEmpty(_CORE_LINUX_LIBS_PATH_ENV): LINUX_LIBS_PATH = $$_CORE_LINUX_LIBS_PATH_ENV

_CORE_WIN_LIBS_PATH_ENV = $$(CORE_WIN_LIBS_PATH)
!isEmpty(_CORE_WIN_LIBS_PATH_ENV):   WIN_LIBS_PATH   = $$_CORE_WIN_LIBS_PATH_ENV

_CORE_WIN_LXERCES_ENV = $$(CORE_WIN_LXERCES_PATH)
!isEmpty(_CORE_WIN_LXERCES_ENV):     WIN_LXERCES_PATH = $$_CORE_WIN_LXERCES_ENV

message("CORE_INSTALL_PREFIX = $$CORE_INSTALL_PREFIX")
message("LINUX_LIBS_PATH     = $$LINUX_LIBS_PATH")
message("WIN_LIBS_PATH       = $$WIN_LIBS_PATH")
