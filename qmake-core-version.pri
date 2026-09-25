QT_MAJOR = $$section(QT_VERSION, ., 0, 0)
wasm {
    contains(QT_MAJOR, 6) {
        LINUX_LIBS_PATH = "/usr/lib/qt6libs_wasm/"
        WIN_LIBS_PATH = "C:/QtCreatorProjects/libs6_wasm"
    } else {
        LINUX_LIBS_PATH = "/usr/lib/qt/"
        WIN_LIBS_PATH = "C:/QtCreatorProjects/libs_wasm"
    }
} else {
    contains(QT_MAJOR, 6) {
        LINUX_LIBS_PATH = "/usr/lib/qt6libs/"
        WIN_LIBS_PATH = "C:/QtCreatorProjects/libs6"
    } else {
        LINUX_LIBS_PATH = "/usr/lib/qt/"
        WIN_LIBS_PATH = "C:/QtCreatorProjects/libs"
    }
}

WIN_LXERCES_PATH = $${WIN_LIBS_PATH}
