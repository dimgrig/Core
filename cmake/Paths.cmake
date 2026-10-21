# Аналог qmake-core-version.pri + core-paths.pri
# CORE_LINUX_LIBS_PATH / CORE_WIN_LIBS_PATH из env, иначе дефолт

if(NOT DEFINED CORE_LINUX_LIBS_PATH)
    if(DEFINED ENV{CORE_LINUX_LIBS_PATH})
        set(CORE_LINUX_LIBS_PATH "$ENV{CORE_LINUX_LIBS_PATH}")
    else()
        if(QT_VERSION_MAJOR EQUAL 6)
            set(CORE_LINUX_LIBS_PATH "/usr/lib/qt6libs")
        else()
            set(CORE_LINUX_LIBS_PATH "/usr/lib/qt")
        endif()
    endif()
endif()

if(NOT DEFINED CORE_WIN_LIBS_PATH)
    if(DEFINED ENV{CORE_WIN_LIBS_PATH})
        set(CORE_WIN_LIBS_PATH "$ENV{CORE_WIN_LIBS_PATH}")
    else()
        if(QT_VERSION_MAJOR EQUAL 6)
            set(CORE_WIN_LIBS_PATH "C:/QtCreatorProjects/libs6")
        else()
            set(CORE_WIN_LIBS_PATH "C:/QtCreatorProjects/libs")
        endif()
    endif()
endif()

if(WIN32)
    set(CORE_DEPLOY_LIBDIR "${CORE_WIN_LIBS_PATH}")
else()
    set(CORE_DEPLOY_LIBDIR "${CORE_LINUX_LIBS_PATH}")
endif()

# Astra / GCC10 (аналог core-gcc10.pri)
if(EXISTS "/etc/astra_version")
    if(DEFINED ENV{ASTRA_GCC_PATH})
        set(ASTRA_GCC_PATH "$ENV{ASTRA_GCC_PATH}")
    else()
        set(ASTRA_GCC_PATH "/home/upkd/opt/gcc-10.1.0")
    endif()
    if(DEFINED ENV{ASTRA_QT_PATH})
        set(ASTRA_QT_PATH "$ENV{ASTRA_QT_PATH}")
    else()
        set(ASTRA_QT_PATH "/opt/qt5.15.12")
    endif()

    set(CMAKE_C_COMPILER   "${ASTRA_GCC_PATH}/bin/gcc")
    set(CMAKE_CXX_COMPILER "${ASTRA_GCC_PATH}/bin/g++")
    set(CMAKE_LINKER       "${ASTRA_GCC_PATH}/bin/g++")

    # moc: передаём нужные дефайны (аналог QMAKE_MOC_FLAGS)
    set(CMAKE_AUTOMOC_MOC_OPTIONS
        "-D__GNUC__=10" "-D__GNUC_MINOR__=3" "-D__GNUC_PATCHLEVEL__=0")

    # include для самого gcc10
    include_directories(SYSTEM
        "${ASTRA_GCC_PATH}/include/c++/10.1.0"
        "${ASTRA_GCC_PATH}/include/c++/10.1.0/x86_64-pc-linux-gnu"
        "${ASTRA_GCC_PATH}/include/c++/10.1.0/backward"
        "${ASTRA_GCC_PATH}/include")
    link_directories("${ASTRA_GCC_PATH}/lib64" "${ASTRA_QT_PATH}/lib")
    message(STATUS "Astra/GCC10: ${ASTRA_GCC_PATH}, Qt: ${ASTRA_QT_PATH}")
endif()