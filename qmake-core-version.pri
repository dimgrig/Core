# --- env override (высший приоритет) ---
# --- env override (высший приоритет) ---
_CORE_LINUX_LIBS_PATH_ENV  = $$(CORE_LINUX_LIBS_PATH)
_CORE_WIN_LIBS_PATH_ENV    = $$(CORE_WIN_LIBS_PATH)
_CORE_WIN_LXERCES_PATH_ENV = $$(CORE_WIN_LXERCES_PATH)

!isEmpty(_CORE_LINUX_LIBS_PATH_ENV):  LINUX_LIBS_PATH   = $$_CORE_LINUX_LIBS_PATH_ENV
!isEmpty(_CORE_WIN_LIBS_PATH_ENV):    WIN_LIBS_PATH     = $$_CORE_WIN_LIBS_PATH_ENV
!isEmpty(_CORE_WIN_LXERCES_PATH_ENV): WIN_LXERCES_PATH  = $$_CORE_WIN_LXERCES_PATH_ENV

# --- дефолты, если env не задан ---
QT_MAJOR = $$section(QT_VERSION, ., 0, 0)
wasm {
    contains(QT_MAJOR, 6) {
        isEmpty(LINUX_LIBS_PATH): LINUX_LIBS_PATH = /usr/lib/qt6libs_wasm/
        isEmpty(WIN_LIBS_PATH):   WIN_LIBS_PATH   = C:/QtCreatorProjects/libs6_wasm
    } else {
        isEmpty(LINUX_LIBS_PATH): LINUX_LIBS_PATH = /usr/lib/qt/
        isEmpty(WIN_LIBS_PATH):   WIN_LIBS_PATH   = C:/QtCreatorProjects/libs_wasm
    }
} else {
    contains(QT_MAJOR, 6) {
        isEmpty(LINUX_LIBS_PATH): LINUX_LIBS_PATH = /usr/lib/qt6libs/
        isEmpty(WIN_LIBS_PATH):   WIN_LIBS_PATH   = C:/QtCreatorProjects/libs6
    } else {
        isEmpty(LINUX_LIBS_PATH): LINUX_LIBS_PATH = /usr/lib/qt/
        isEmpty(WIN_LIBS_PATH):   WIN_LIBS_PATH   = C:/QtCreatorProjects/libs
    }
}

isEmpty(WIN_LXERCES_PATH): WIN_LXERCES_PATH = $$WIN_LIBS_PATH
