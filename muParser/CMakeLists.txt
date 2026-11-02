file(GLOB_RECURSE MUPARSER_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE MUPARSER_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

if(NOT MUPARSER_SOURCES)
    message(FATAL_ERROR "muParser: не найдено *.cpp в ${CMAKE_CURRENT_SOURCE_DIR}/src")
endif()

# STATIC указан явно — BUILD_SHARED_LIBS не перебьёт
add_library(muParser STATIC ${MUPARSER_SOURCES} ${MUPARSER_HEADERS})
add_library(Core::muParser ALIAS muParser)

# ── Windows: Qt/mingw headers могут протаскивать _UNICODE в consumer builds ──
# ── (MathParser.cpp получает wide-версию DefineOprt/AddCallback/SetExpr),   ──
# ── а сам muParser собирается narrow → mangling mismatch → undefined ref.    ──
# ── Принудительно делаем muParser wide через PUBLIC, чтобы и библиотека,     ──
# ── и все её потребители видели одинаковый mu::string_type.                  ──
if(WIN32)
    target_compile_definitions(muParser PUBLIC _UNICODE UNICODE)
endif()

# PUBLIC: MUPARSER_STATIC нужен и потребителям (MathParser.h → muParser.h)
target_compile_definitions(muParser PUBLIC MUPARSER_STATIC)

target_include_directories(muParser PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CORE_INSTALL_INCLUDEDIR}/muParser>)

# RPATH/PIC — на случай линковки из shared-либ
set_target_properties(muParser PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON)

install(TARGETS muParser
    EXPORT CoreTargets
    ARCHIVE DESTINATION "${CORE_INSTALL_LIBDIR}")

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
    DESTINATION "${CORE_INSTALL_INCLUDEDIR}/muParser"
    FILES_MATCHING PATTERN "*.h")
