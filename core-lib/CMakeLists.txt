core_add_library(core-lib
    QT Core Gui PrintSupport
    DEFINES CORELIB_LIBRARY
)

target_sources(core-lib PRIVATE
    CoreConfig.cpp
    Logger.cpp
    TestRunner.cpp
    enum_macro.cpp
    mixins.cpp
    Profiler.cpp
    Sleeper.cpp
    macroses.cpp
    templates.cpp
    templates_cout.cpp

    CoreConfig.h
    Logger.h
    Random.h
    TestRunner.h
    enum_macro.h
    mixins.h
    Profiler.h
    Sleeper.h
    bit.h
    interfaces.h
    macroses.h
    templates.h
    templates_cout.h
    visitor.h
)

