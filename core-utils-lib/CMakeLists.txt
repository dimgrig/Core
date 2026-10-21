core_add_library(core-utils-lib
    QT Core PrintSupport Network
    DEPENDS core-lib core-journal-lib core-widgets-lib muParser
    DEFINES MUPARSER_STATIC
)

target_sources(core-utils-lib PRIVATE
    AsyncWaiter.cpp
    CQueue.cpp
    Calibration.cpp
    DelayUtils.cpp
    DocxExporter.cpp
    FilterMixin.cpp
    MathParser.cpp
    SessionCache.cpp
    functions.cpp
    functions_degrees.cpp
    functions_http.cpp

    AsyncWaiter.h
    CQueue.h
    Calibration.h
    DelayUtils.h
    DocxExporter.h
    FilterMixin.h
    MathParser.h
    SessionCache.h
    dimensions.h
    functions.h
    core-utils-lib-global.h
    functions_degrees.h
    functions_http.h
)

