# core-deploy.pri — POST_LINK копирование артефактов в install-layout.
# Читает: CORE_INSTALL_PREFIX, CORE_INSTALL_{LIB,BIN}DIR, CORE_APP_LOCAL_DIR,
#         CORE_DEPLOY_TARGET (деф. $$TARGET), CORE_DEPLOY_PREFIX (деф. lib).
# Требует: DESTINATION_PATH (из qmake-destination-path.pri).

isEmpty(CORE_DEPLOY_TARGET): CORE_DEPLOY_TARGET = $$TARGET
isEmpty(CORE_DEPLOY_PREFIX): CORE_DEPLOY_PREFIX = lib
isEmpty(CORE_DEPLOY_SRC):    CORE_DEPLOY_SRC    = $$_PRO_FILE_PWD_/../../$${CORE_DEPLOY_TARGET}-binaries/$${DESTINATION_PATH}

# ────────────────────────────────────────────────────────────
#  Linux / macOS
# ────────────────────────────────────────────────────────────
unix:!macx|macx {
    _DEST_LIB     = $$CORE_INSTALL_PREFIX/$$CORE_INSTALL_LIBDIR
    _DEST_INCLUDE = $$CORE_INSTALL_PREFIX/$$CORE_INSTALL_INCLUDEDIR/$${CORE_DEPLOY_TARGET}

    _CORE_DEPLOY_CMDS =
    _CORE_DEPLOY_CMDS += "mkdir -p $$shell_path($$_DEST_LIB) $$shell_path($$_DEST_INCLUDE)"

    unix:!macx {
        contains(CONFIG, staticlib) {
            _CORE_DEPLOY_CMDS += "cp -a $$shell_path($${CORE_DEPLOY_SRC})/*.a $$shell_path($$_DEST_LIB)/"
        } else {
            _CORE_DEPLOY_CMDS += "cp -a $$shell_path($${CORE_DEPLOY_SRC})/*.so* $$shell_path($$_DEST_LIB)/"
        }
    }
    macx {
        contains(CONFIG, staticlib) {
            _CORE_DEPLOY_CMDS += "cp -a $$shell_path($${CORE_DEPLOY_SRC})/*.a $$shell_path($$_DEST_LIB)/"
        } else {
            _CORE_DEPLOY_CMDS += "cp -a $$shell_path($${CORE_DEPLOY_SRC})/*.dylib* $$shell_path($$_DEST_LIB)/"
        }
    }

    for(_hdr, HEADERS) {
        _hdr_abs = $$absolute_path($$_hdr, $$_PRO_FILE_PWD_)
        _hdr_rel = $$relative_path($$_hdr_abs, $$_PRO_FILE_PWD_)
        _hdr_rel ~= s,^include/,,
        _hdr_dir = $$dirname($$_hdr_rel)
        !equals(_hdr_dir, ".") {
            _CORE_DEPLOY_CMDS += "mkdir -p $$shell_path($$_DEST_INCLUDE/$$_hdr_dir)"
        }
        _CORE_DEPLOY_CMDS += "cp -a $$shell_path($$_hdr_abs) $$shell_path($$_DEST_INCLUDE/$$_hdr_dir)"
    }

    _LATEST = $$dirname($$CORE_INSTALL_PREFIX)/latest
    !isEmpty(CORE_INSTALL_PREFIX) {
        _LATEST = $$clean_path($$CORE_INSTALL_PREFIX/../latest)
        !equals(_LATEST, $$CORE_INSTALL_PREFIX) {
            _CORE_DEPLOY_CMDS += "ln -sfn $$shell_path($$CORE_INSTALL_PREFIX) $$shell_path($$_LATEST)"
            message("core-deploy: latest -> [$$_LATEST]")
        }
    }

    QMAKE_POST_LINK += $$join(_CORE_DEPLOY_CMDS, " && ")
}

# ────────────────────────────────────────────────────────────
#  Windows (native + cross)
# ────────────────────────────────────────────────────────────
win32|win64 {
    _DEST_LIB     = $$CORE_APP_LOCAL_DIR/$$CORE_INSTALL_LIBDIR
    _DEST_BIN     = $$CORE_APP_LOCAL_DIR/$$CORE_INSTALL_BINDIR
    _DEST_INCLUDE = $$CORE_APP_LOCAL_DIR/$$CORE_INSTALL_INCLUDEDIR/$${CORE_DEPLOY_TARGET}

    _CORE_DEPLOY_CMDS =
    _CORE_DEPLOY_CMDS += "mkdir -p $$shell_path($$_DEST_LIB) $$shell_path($$_DEST_BIN) $$shell_path($$_DEST_INCLUDE)"

    contains(CONFIG, staticlib) {
        _CORE_DEPLOY_CMDS += "cp -a $$quote($$shell_path($${CORE_DEPLOY_SRC}/$${CORE_DEPLOY_PREFIX}$${CORE_DEPLOY_TARGET}.a)) $$quote($$shell_path($$_DEST_LIB))/"
    } else {
        _CORE_DEPLOY_CMDS += "cp -a $$quote($$shell_path($${CORE_DEPLOY_SRC}/$${CORE_DEPLOY_TARGET}.dll)) $$quote($$shell_path($$_DEST_BIN))/"
        _CORE_DEPLOY_CMDS += "cp -a $$quote($$shell_path($${CORE_DEPLOY_SRC}/$${CORE_DEPLOY_PREFIX}$${CORE_DEPLOY_TARGET}.a)) $$quote($$shell_path($$_DEST_LIB))/"
    }

    for(_hdr, HEADERS) {
        _hdr_abs = $$absolute_path($$_hdr, $$_PRO_FILE_PWD_)
        _hdr_rel = $$relative_path($$_hdr_abs, $$_PRO_FILE_PWD_)
        _hdr_rel ~= s,^include/,,
        _hdr_dir = $$dirname($$_hdr_rel)
        !equals(_hdr_dir, ".") {
            _CORE_DEPLOY_CMDS += "mkdir -p $$shell_path($$_DEST_INCLUDE/$$_hdr_dir)"
        }
        _CORE_DEPLOY_CMDS += "cp -a $$shell_path($$_hdr_abs) $$shell_path($$_DEST_INCLUDE/$$_hdr_dir)"
    }

    _LATEST = $$dirname($$CORE_APP_LOCAL_DIR)/latest
    !isEmpty(CORE_APP_LOCAL_DIR) {
        _LATEST = $$clean_path($$CORE_APP_LOCAL_DIR/../latest)
        !equals(_LATEST, $$CORE_APP_LOCAL_DIR) {
            _CORE_DEPLOY_CMDS += "cmake -DLATEST=$$shell_quote($$clean_path($$_LATEST)) -DTARGET=$$shell_quote($$clean_path($$CORE_APP_LOCAL_DIR)) -P $$clean_path($$CORE_ROOT/cmake/CreateLatestLink.cmake)"
            message("core-deploy: latest -> [$$_LATEST] (junction via cmake -P)")
        }
    }

    QMAKE_POST_LINK += $$join(_CORE_DEPLOY_CMDS, " && ")
}
