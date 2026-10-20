# core-deploy.pri
# Добавляет QMAKE_POST_LINK для копирования .so/.dll в LINUX_LIBS_PATH/WIN_LIBS_PATH.
# Читает: CORE_DEPLOY_TARGET (по умолчанию $$TARGET), CORE_DEPLOY_PREFIX (по умолчанию "lib").
# Требует: DESTINATION_PATH, LINUX_LIBS_PATH, WIN_LIBS_PATH.

isEmpty(CORE_DEPLOY_TARGET): CORE_DEPLOY_TARGET = $$TARGET
isEmpty(CORE_DEPLOY_PREFIX): CORE_DEPLOY_PREFIX = lib
isEmpty(CORE_DEPLOY_SRC):    CORE_DEPLOY_SRC    = $$_PRO_FILE_PWD_/../../$${CORE_DEPLOY_TARGET}-binaries/$${DESTINATION_PATH}

# unix: создаём каталог и обеспечиваем запись, затем копируем glob-ом
unix {
    #но без sudo не создастся в системно месте!
    QMAKE_POST_LINK += mkdir -p $$shell_path($${LINUX_LIBS_PATH}) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += chmod u+w $$shell_path($${LINUX_LIBS_PATH}) $$escape_expand(\\n\\t)

    contains(CONFIG, staticlib) {
        QMAKE_POST_LINK += cp -a $$shell_path($${CORE_DEPLOY_SRC})/*.a $$shell_path($${LINUX_LIBS_PATH})/ $$escape_expand(\\n\\t)
    } else {
        QMAKE_POST_LINK += cp -a $$shell_path($${CORE_DEPLOY_SRC})/*.so* $$shell_path($${LINUX_LIBS_PATH})/ $$escape_expand(\\n\\t)
    }
}

# Windows: создаём каталог через cmd.exe, затем копируем
win32|win64 {
    QMAKE_POST_LINK += if not exist $$quote($$shell_path($${WIN_LIBS_PATH})) mkdir $$quote($$shell_path($${WIN_LIBS_PATH})) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($${CORE_DEPLOY_SRC}/$${CORE_DEPLOY_TARGET}*.dll)) $$quote($$shell_path($${WIN_LIBS_PATH})) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($${CORE_DEPLOY_SRC}/$${CORE_DEPLOY_PREFIX}$${CORE_DEPLOY_TARGET}*.a)) $$quote($$shell_path($${WIN_LIBS_PATH})) $$escape_expand(\\n\\t)
}
