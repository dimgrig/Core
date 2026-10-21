QT       += core
greaterThan(QT_MAJOR_VERSION, 4): QT += gui widgets sql printsupport

TEMPLATE = lib
DEFINES += CORE_MVC_LIB_LIBRARY
CONFIG += c++17

INCLUDEPATH += "$$_PRO_FILE_PWD_/"

CORE_LINK_LIBS = core-lib core-journal-lib core-utils-lib core-widgets-lib
include(../core-build.pri)
VERSION = $$CORE_VERSION

unix { target.path = /usr/lib/ }
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    BaseEditTableModel.h \
    BaseEditTableView.h \
    BaseSqlRelationalTableModel.h \
    BaseTableModel.h \
    BaseTableView.h \
    CleanupManager.h \
    Completer.h \
    DateDelegate.h \
    DateTimeUTCDelegate.h \
    DoubleSpinBoxDelegate.h \
    EditBar.h \
    EnumFilterWidget.h \
    FilterHeader.h \
    LineEditDelegate.h \
    ListEditDelegate.h \
    LogLevelDelegate.h \
    LookupDelegate.h \
    NumericFilterWidget.h \
    NumericIdProxyModel.h \
    ReadOnlyDelegate.h \
    SpinBoxDelegate.h \
    StringIdProxyModel.h \
    TableShortcutHandler.h \
    TemplateEditRelationalTableModel.h \
    TemplateEditRelationalTableModel.hpp \
    TemplateEditTableModel.h \
    TemplateEditTableModel.hpp \
    UniversalFormWidget.h \
    ValueListDelegate.h \
    core-mvc-lib-global.h \

SOURCES += \
    BaseEditTableModel.cpp \
    BaseEditTableView.cpp \
    BaseSqlRelationalTableModel.cpp \
    BaseTableModel.cpp \
    BaseTableView.cpp \
    CleanupManager.cpp \
    Completer.cpp \
    DateDelegate.cpp \
    DateTimeUTCDelegate.cpp \
    DoubleSpinBoxDelegate.cpp \
    EditBar.cpp \
    EnumFilterWidget.cpp \
    FilterHeader.cpp \
    LineEditDelegate.cpp \
    ListEditDelegate.cpp \
    LogLevelDelegate.cpp \
    LookupDelegate.cpp \
    NumericFilterWidget.cpp \
    NumericIdProxyModel.cpp \
    ReadOnlyDelegate.cpp \
    SpinBoxDelegate.cpp \
    StringIdProxyModel.cpp \
    TableShortcutHandler.cpp \
    TemplateEditRelationalTableModel.cpp \
    TemplateEditTableModel.cpp \
    UniversalFormWidget.cpp \
    ValueListDelegate.cpp
