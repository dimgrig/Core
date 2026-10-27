core_add_library(qcustomplot
    QT      Core Gui Widgets PrintSupport OpenGL
    VERSION 2.1.1
    DEFINES QCUSTOMPLOT_COMPILE_LIBRARY QCUSTOMPLOT_USE_OPENGL
)

target_sources(qcustomplot PRIVATE
    qcustomplot.cpp
    qcustomplot.h
)

# Потребителям: USE_LIBRARY (вместо COMPILE_LIBRARY) и OPENGL
target_compile_definitions(qcustomplot INTERFACE
    QCUSTOMPLOT_USE_LIBRARY
    QCUSTOMPLOT_USE_OPENGL
)

if(WIN32)
    target_link_libraries(qcustomplot PUBLIC opengl32 glu32)
endif()
