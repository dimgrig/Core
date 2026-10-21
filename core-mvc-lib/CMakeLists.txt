core_add_library(core-mvc-lib
    QT Core Gui Widgets Sql PrintSupport
    DEPENDS core-lib core-journal-lib core-utils-lib core-widgets-lib
)

target_sources(core-mvc-lib PRIVATE
    BaseEditTableModel.cpp
    BaseEditTableView.cpp
    BaseSqlRelationalTableModel.cpp
    BaseTableModel.cpp
    BaseTableView.cpp
    CleanupManager.cpp
    Completer.cpp
    DateDelegate.cpp
    DateTimeUTCDelegate.cpp
    DoubleSpinBoxDelegate.cpp
    EditBar.cpp
    EnumFilterWidget.cpp
    FilterHeader.cpp
    LineEditDelegate.cpp
    ListEditDelegate.cpp
    LogLevelDelegate.cpp
    LookupDelegate.cpp
    NumericFilterWidget.cpp
    NumericIdProxyModel.cpp
    ReadOnlyDelegate.cpp
    SpinBoxDelegate.cpp
    StringIdProxyModel.cpp
    TableShortcutHandler.cpp
    TemplateEditRelationalTableModel.cpp
    TemplateEditTableModel.cpp
    UniversalFormWidget.cpp
    ValueListDelegate.cpp

    BaseEditTableModel.h
    BaseEditTableView.h
    BaseSqlRelationalTableModel.h
    BaseTableModel.h
    BaseTableView.h
    CleanupManager.h
    Completer.h
    DateDelegate.h
    DateTimeUTCDelegate.h
    DoubleSpinBoxDelegate.h
    EditBar.h
    EnumFilterWidget.h
    FilterHeader.h
    LineEditDelegate.h
    ListEditDelegate.h
    LogLevelDelegate.h
    LookupDelegate.h
    NumericFilterWidget.h
    NumericIdProxyModel.h
    ReadOnlyDelegate.h
    SpinBoxDelegate.h
    StringIdProxyModel.h
    TableShortcutHandler.h
    TemplateEditRelationalTableModel.h
    TemplateEditRelationalTableModel.hpp
    TemplateEditTableModel.h
    TemplateEditTableModel.hpp
    UniversalFormWidget.h
    ValueListDelegate.h
    core-mvc-lib-global.h
)

