core_add_library(core-widgets-lib
    QT Core Gui Widgets Svg #StateMachine
    DEPENDS core-lib core-journal-lib
)

target_sources(core-widgets-lib PRIVATE
    AboutPanel.cpp
    Approximation.cpp
    AspectRatioPixmapLabel.cpp
    Auth.cpp
    CalendarWidget.cpp
    ComboBoxDialog.cpp
    ConnectionButton.cpp
    DateTimeRangeWidget.cpp
    DetailListWidget.cpp
    DoubleSpinBoxDialog.cpp
    DynamicListPanel.cpp
    DynamicListWidget.cpp
    FlowLayout.cpp
    FocusWatcher.cpp
    IUserRoleManager.cpp
    IconManager.cpp
    MenuBar.cpp
    MenuCornerWidget.cpp
    MessageBox.cpp
    MockUserRoleManager.cpp
    MultiSelectWidget.cpp
    RotatedLabel.cpp
    RoundedLabel.cpp
    TabBarHorizontalText.cpp
    User.cpp
    aqp.cpp

    AboutPanel.h
    Approximation.h
    AspectRatioPixmapLabel.h
    Auth.h
    CalendarWidget.h
    ComboBoxDialog.h
    ConnectionButton.h
    DateTimeRangeWidget.h
    DetailListWidget.h
    DoubleSpinBoxDialog.h
    DynamicListPanel.h
    DynamicListWidget.h
    FlowLayout.h
    FocusWatcher.h
    IUserRoleManager.h
    IconManager.h
    MenuBar.h
    MenuCornerWidget.h
    MessageBox.h
    MockUserRoleManager.h
    MultiSelectWidget.h
    RotatedLabel.h
    RoundedLabel.h
    SpinBoxEditor.h
    TabBarHorizontalText.h
    User.h
    aqp.h
    core-widgets-lib-global.h
    icons.h

    uis/Auth.ui
    uis/User.ui
    uis/Users.ui
    uis/about.ui
    uis/combobox_dialog.ui
    uis/doublespinbox_dialog.ui
    uis/menu_corner_widget.ui
    uis/settings.ui
)

