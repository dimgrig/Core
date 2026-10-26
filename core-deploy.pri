# core-deploy.pri — POST_LINK копирование артефактов в install-layout.
# Читает: CORE_INSTALL_PREFIX, CORE_INSTALL_{LIB,BIN}DIR, CORE_APP_LOCAL_DIR,
#         CORE_DEPLOY_TARGET (деф. $$TARGET), CORE_DEPLOY_PREFIX (деф. lib).
# Требует: DESTINATION_PATH (из qmake-destination-path.pri).

isEmpty(CORE_DEPLOY_TARGET): CORE_DEPLOY_TARGET = $$TARGET
isEmpty(CORE_DEPLOY_PREFIX): CORE_DEPLOY_PREFIX = lib
isEmpty(CORE_DEPLOY_SRC):    CORE_DEPLOY_SRC    = $$_PRO_FILE_PWD_/../../$${CORE_DEPLOY_TARGET}-binaries/$${DESTINATION_PATH}

isEmpty(CORE_DEPLOY_TARGET): CORE_DEPLOY_TARGET = $$TARGET
isEmpty(CORE_DEPLOY_PREFIX): CORE_DEPLOY_PREFIX = lib
isEmpty(CORE_DEPLOY_SRC):    CORE_DEPLOY_SRC    = $$_PRO_FILE_PWD_/../../$${CORE_DEPLOY_TARGET}-binaries/$${DESTINATION_PATH}

# ────────────────────────────────────────────────────────────
#  Linux / macOS
# ────────────────────────────────────────────────────────────
unix:!macx|macx {
    _DEST_LIB     = $$CORE_INSTALL_PREFIX/$$CORE_INSTALL_LIBDIR
    _DEST_INCLUDE = $$CORE_INSTALL_PREFIX/$$CORE_INSTALL_INCLUDEDIR/$${CORE_DEPLOY_TARGET}

    QMAKE_POST_LINK += mkdir -p $$shell_path($$_DEST_LIB) $$shell_path($$_DEST_INCLUDE) $$escape_expand(\\n\\t)

    unix:!macx {
        contains(CONFIG, staticlib) {
            QMAKE_POST_LINK += cp -a $$shell_path($${CORE_DEPLOY_SRC})/*.a $$shell_path($$_DEST_LIB)/ 2>/dev/null || true $$escape_expand(\\n\\t)
        } else {
            QMAKE_POST_LINK += cp -a $$shell_path($${CORE_DEPLOY_SRC})/*.so* $$shell_path($$_DEST_LIB)/ 2>/dev/null || true $$escape_expand(\\n\\t)
        }
    }
    macx {
        contains(CONFIG, staticlib) {
            QMAKE_POST_LINK += cp -a $$shell_path($${CORE_DEPLOY_SRC})/*.a $$shell_path($$_DEST_LIB)/ 2>/dev/null || true $$escape_expand(\\n\\t)
        } else {
            QMAKE_POST_LINK += cp -a $$shell_path($${CORE_DEPLOY_SRC})/*.dylib* $$shell_path($$_DEST_LIB)/ 2>/dev/null || true $$escape_expand(\\n\\t)
        }
    }

    for(_hdr, HEADERS) {
        # Нормализуем к абсолютному пути: работает и для относительных
        # ("Logger.h", "bindings/Foo.h"), и для абсолютных
        # ($$files($$_PRO_FILE_PWD_/include/*.h)).
        _hdr_abs = $$absolute_path($$_hdr, $$_PRO_FILE_PWD_)

        # Относительный путь для destination. Заголовки под include/ ставим плоско
        # (паритет с CMake install: FILE ... ).
        _hdr_rel = $$relative_path($$_hdr_abs, $$_PRO_FILE_PWD_)
        _hdr_rel ~= s,^include/,,

        _hdr_dir = $$dirname($$_hdr_rel)
        !equals(_hdr_dir, ".") {
            QMAKE_POST_LINK += mkdir -p $$shell_path($$_DEST_INCLUDE/$$_hdr_dir) $$escape_expand(\\n\\t)
        }
        QMAKE_POST_LINK += cp -a $$shell_path($$_hdr_abs) $$shell_path($$_DEST_INCLUDE/$$_hdr_dir) $$escape_expand(\\n\\t)
    }

    _LATEST = $$dirname($$CORE_INSTALL_PREFIX)/latest
    !isEmpty(CORE_INSTALL_PREFIX) {
        _LATEST = $$clean_path($$CORE_INSTALL_PREFIX/../latest)
        !equals(_LATEST, $$CORE_INSTALL_PREFIX) {
            QMAKE_POST_LINK += ln -sfn $$shell_path($$CORE_INSTALL_PREFIX) $$shell_path($$_LATEST) $$escape_expand(\\n\\t)
            message("core-deploy: latest -> [$$_LATEST]")
        }
    }
}

# ────────────────────────────────────────────────────────────
#  Windows (native + cross)
# ────────────────────────────────────────────────────────────
win32|win64 {
    _DEST_LIB     = $$CORE_APP_LOCAL_DIR/$$CORE_INSTALL_LIBDIR
    _DEST_BIN     = $$CORE_APP_LOCAL_DIR/$$CORE_INSTALL_BINDIR
    _DEST_INCLUDE = $$CORE_APP_LOCAL_DIR/$$CORE_INSTALL_INCLUDEDIR/$${CORE_DEPLOY_TARGET}

    QMAKE_POST_LINK += mkdir -p $$shell_path($$_DEST_LIB) $$shell_path($$_DEST_BIN) $$shell_path($$_DEST_INCLUDE) 2>/dev/null || true $$escape_expand(\\n\\t)

    contains(CONFIG, staticlib) {
        QMAKE_POST_LINK += cp -a $$quote($$shell_path($${CORE_DEPLOY_SRC}/$${CORE_DEPLOY_PREFIX}$${CORE_DEPLOY_TARGET}.a)) $$quote($$shell_path($$_DEST_LIB))/ 2>/dev/null || true $$escape_expand(\\n\\t)
    } else {
        QMAKE_POST_LINK += cp -a $$quote($$shell_path($${CORE_DEPLOY_SRC}/$${CORE_DEPLOY_TARGET}.dll)) $$quote($$shell_path($$_DEST_BIN))/ 2>/dev/null || true $$escape_expand(\\n\\t)
        QMAKE_POST_LINK += cp -a $$quote($$shell_path($${CORE_DEPLOY_SRC}/$${CORE_DEPLOY_PREFIX}$${CORE_DEPLOY_TARGET}.a)) $$quote($$shell_path($$_DEST_LIB))/ 2>/dev/null || true $$escape_expand(\\n\\t)
    }

    for(_hdr, HEADERS) {
        # Нормализуем к абсолютному пути: работает и для относительных
        # ("Logger.h", "bindings/Foo.h"), и для абсолютных
        # ($$files($$_PRO_FILE_PWD_/include/*.h)).
        _hdr_abs = $$absolute_path($$_hdr, $$_PRO_FILE_PWD_)

        # Относительный путь для destination. Заголовки под include/ ставим плоско
        # (паритет с CMake install: FILE ... ).
        _hdr_rel = $$relative_path($$_hdr_abs, $$_PRO_FILE_PWD_)
        _hdr_rel ~= s,^include/,,

        _hdr_dir = $$dirname($$_hdr_rel)
        !equals(_hdr_dir, ".") {
            QMAKE_POST_LINK += mkdir -p $$shell_path($$_DEST_INCLUDE/$$_hdr_dir) $$escape_expand(\\n\\t)
        }
        QMAKE_POST_LINK += cp -a $$shell_path($$_hdr_abs) $$shell_path($$_DEST_INCLUDE/$$_hdr_dir) $$escape_expand(\\n\\t)
    }

    _LATEST = $$dirname($$CORE_APP_LOCAL_DIR)/latest
    !isEmpty(CORE_APP_LOCAL_DIR) {
        _LATEST = $$clean_path($$CORE_APP_LOCAL_DIR/../latest)
        !equals(_LATEST, $$CORE_APP_LOCAL_DIR) {
            QMAKE_POST_LINK += cmd /c "if exist $$shell_path($$_LATEST) rmdir $$shell_path($$_LATEST) 2>nul & mklink /J $$shell_path($$_LATEST) $$shell_path($$CORE_APP_LOCAL_DIR)" $$escape_expand(\\n\\t)
            message("core-deploy: latest -> [$$_LATEST]")
        }
    }
}
