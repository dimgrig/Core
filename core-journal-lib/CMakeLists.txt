core_add_library(core-journal-lib
    QT Core PrintSupport
    DEPENDS core-lib
)

target_sources(core-journal-lib PRIVATE
    JournalAccess.cpp
    JournalBase.cpp

    Categories.h
    JournalAccess.h
    JournalBase.h
    core-journal-lib-global.h
)

