core_add_library(core-utils-lib
    QT Core Network
    DEPENDS core-lib core-journal-lib muParser
    DEFINES MUPARSER_STATIC
)

target_sources(core-utils-lib PRIVATE
    CQueue.cpp
    Calibration.cpp
    DelayUtils.cpp
    FilterMixin.cpp
    MathParser.cpp
    SessionCache.cpp
    functions.cpp
    functions_degrees.cpp
    functions_http.cpp

    CQueue.h
    Calibration.h
    DelayUtils.h
    FilterMixin.h
    MathParser.h
    SessionCache.h
    dimensions.h
    functions.h
    core-utils-lib-global.h
    functions_degrees.h
    functions_http.h
)

