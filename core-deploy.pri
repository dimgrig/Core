# core-deploy.pri — POST_LINK копирование артефактов в install-layout.
# Читает: CORE_INSTALL_PREFIX, CORE_INSTALL_{LIB,BIN}DIR, CORE_APP_LOCAL_DIR,
#         CORE_DEPLOY_TARGET (деф. $$TARGET), CORE_DEPLOY_PREFIX (деф. lib).
# Требует: DESTINATION_PATH (из qmake-destination-path.pri).

isEmpty(CORE_DEPLOY_TARGET): CORE_DEPLOY_TARGET = $$TARGET
isEmpty(CORE_DEPLOY_PREFIX): CORE_DEPLOY_PREFIX = lib
isEmpty(CORE_DEPLOY_SRC):    CORE_DEPLOY_SRC    = $$_PRO_FILE_PWD_/../../$${CORE_DEPLOY_TARGET}-binaries/$${DESTINATION_PATH}

# ── Linux ──────────────────────────────────────────────────
unix:!macx {
    _DEST_LIB = $$CORE_INSTALL_PREFIX/$$CORE_INSTALL_LIBDIR
    QMAKE_POST_LINK += mkdir -p $$shell_path($$_DEST_LIB) 2>/dev/null || true $$escape_expand(\\n\\t)
    contains(CONFIG, staticlib) {
        QMAKE_POST_LINK += cp -a $$shell_path($${CORE_DEPLOY_SRC})/*.a $$shell_path($$_DEST_LIB)/ 2>/dev/null || true $$escape_expand(\\n\\t)
    } else {
        QMAKE_POST_LINK += cp -a $$shell_path($${CORE_DEPLOY_SRC})/*.so* $$shell_path($$_DEST_LIB)/ 2>/dev/null || true $$escape_expand(\\n\\t)
    }
}

# ── Windows (нативный + кросс из Linux) ────────────────────
# .dll → bin/, import-lib .a → lib/ (как в CMake).
win32|win64 {
    _DEST_LIB = $$CORE_APP_LOCAL_DIR/$$CORE_INSTALL_LIBDIR
    _DEST_BIN = $$CORE_APP_LOCAL_DIR/$$CORE_INSTALL_BINDIR

    QMAKE_POST_LINK += mkdir -p $$shell_path($$_DEST_LIB) $$shell_path($$_DEST_BIN) 2>/dev/null || true $$escape_expand(\\n\\t)

    contains(CONFIG, staticlib) {
        QMAKE_POST_LINK += cp -a $$quote($$shell_path($${CORE_DEPLOY_SRC}/$${CORE_DEPLOY_PREFIX}$${CORE_DEPLOY_TARGET}*.a)) $$quote($$shell_path($$_DEST_LIB))/ 2>/dev/null || true $$escape_expand(\\n\\t)
    } else {
        QMAKE_POST_LINK += cp -a $$quote($$shell_path($${CORE_DEPLOY_SRC}/$${CORE_DEPLOY_TARGET}*.dll)) $$quote($$shell_path($$_DEST_BIN))/ 2>/dev/null || true $$escape_expand(\\n\\t)
        QMAKE_POST_LINK += cp -a $$quote($$shell_path($${CORE_DEPLOY_SRC}/$${CORE_DEPLOY_PREFIX}$${CORE_DEPLOY_TARGET}*.a)) $$quote($$shell_path($$_DEST_LIB))/ 2>/dev/null || true $$escape_expand(\\n\\t)
    }
}
