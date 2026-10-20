# core-astra.pri
# Astra Linux / GCC 10: подключение core-gcc10.pri, если система Astra.
# На других системах ничего не делает.

exists(/etc/astra_version) {
    DISTFILES += $$CORE_ROOT/core-gcc10.pri
    include($$CORE_ROOT/core-gcc10.pri)
    message($${ASTRA_GCC_PATH})
    message($${ASTRA_QT_PATH})
}