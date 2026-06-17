RV_TYPE = "TESTRV"

#DEFINES += OPI

#DEFINES += MI E2010_MI
#DEFINES += MI STM32_MI

contains(DEFINES, "OPI") {
    DEFINES += QPROCESS_GPIO
    #DEFINES += OPI_DIO
}

#contains(RMO_TYPE, "FET2") {
#    DEFINES += FET2
#    DEFINES += FET
#}

#contains(RMO_TYPE, "FET5") {
#    DEFINES += FET5
#    DEFINES += FET
#}

contains(RMO_TYPE, "TESTRV") {
    DEFINES += TESTRV
}

LINUX_LIBS_PATH = "/usr/lib/qt/"
WIN_LIBS_PATH = "C:/QtCreatorProjects/libs"
WIN_LXERCES_PATH = $${WIN_LIBS_PATH}
