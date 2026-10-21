# core-astra.pri
# Astra Linux / GCC 10: подключение core-gcc10.pri, если система Astra.
# На других системах ничего не делает.

exists(/etc/astra_version) {
    _ASTRA_GCC_ENV = $$(ASTRA_GCC_PATH)
    !isEmpty(_ASTRA_GCC_ENV) {
        DISTFILES += $$CORE_ROOT/core-gcc10.pri
        include($$CORE_ROOT/core-gcc10.pri)
        message("Astra/GCC10: ASTRA_GCC_PATH=$$ASTRA_GCC_PATH, ASTRA_QT_PATH=$$ASTRA_QT_PATH")
    } else {
        message("Astra detected, ASTRA_GCC_PATH not set — using default toolchain.")
    }
}
