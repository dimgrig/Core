if(QT_VERSION_MAJOR EQUAL 6)
    core_add_library(core-widgets-lib
        QT Core Gui Widgets Svg StateMachine SerialPort SerialBus Mqtt
        DEPENDS core-lib core-journal-lib core-modbus-lib core-io-lib
        INCLUDES bindings
    )
    qt_add_resources(CORE_WIDGETS_RESOURCES_QRC
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/core_widgets_resources.qrc)
else()
    core_add_library(core-widgets-lib
        QT Core Gui Widgets Svg SerialPort SerialBus Mqtt #StateMachine
        DEPENDS core-lib core-journal-lib core-modbus-lib core-io-lib
        INCLUDES bindings
    )
    qt5_add_resources(CORE_WIDGETS_RESOURCES_QRC
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/core_widgets_resources.qrc)
endif()

target_sources(core-widgets-lib PRIVATE
    ${CORE_WIDGETS_RESOURCES_QRC}
    AboutPanel.cpp
    Approximation.cpp
    AspectRatioPixmapLabel.cpp
    AsyncWaiter.cpp
    Auth.cpp
    CalendarWidget.cpp
    ComboBoxDialog.cpp
    ConnectionButton.cpp
    DateTimeRangeWidget.cpp
    DetailListWidget.cpp
    DocxExporter.cpp
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
    ModbusSettingsUI.cpp
    MultiSelectWidget.cpp
    RotatedLabel.cpp
    RoundedLabel.cpp
    SCPISettingsUI.cpp
    TabBarHorizontalText.cpp
    User.cpp
    WidgetResizeMixin.cpp
    aqp.cpp
    bindings/AdvancedClickablePixmapLabelBinding.cpp
    bindings/BindingFactory.cpp
    bindings/BindingManager.cpp
    bindings/BitAwareBindingManager.cpp
    bindings/CheckBoxBinding.cpp
    bindings/ClickablePixmapLabelBinding.cpp
    bindings/ComboBoxBinding.cpp
    bindings/DoubleSpinBoxBinding.cpp
    bindings/LabelBinding.cpp
    bindings/LineEditBinding.cpp
    bindings/PixmapLabelBinding.cpp
    bindings/RegisterBinding.cpp
    bindings/SpinBoxBinding.cpp
    functions_widgets.cpp

    AboutPanel.h
    Approximation.h
    AsyncWaiter.h
    AspectRatioPixmapLabel.h
    Auth.h
    CalendarWidget.h
    ComboBoxDialog.h
    ConnectionButton.h
    DateTimeRangeWidget.h
    DetailListWidget.h
    DocxExporter.h
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
    ModbusSettingsUI.h
    MultiSelectWidget.h
    RotatedLabel.h
    RoundedLabel.h
    SCPISettingsUI.h
    SpinBoxEditor.h
    TabBarHorizontalText.h
    User.h
    WidgetResizeMixin.h
    aqp.h
    core-widgets-lib-global.h
    icons.h    
    bindings/AdvancedClickablePixmapLabelBinding.h
    bindings/BindingFactory.h
    bindings/BindingManager.h
    bindings/BitAwareBindingManager.h
    bindings/CheckBoxBinding.h
    bindings/ClickablePixmapLabelBinding.h
    bindings/ComboBoxBinding.h
    bindings/DoubleSpinBoxBinding.h
    bindings/LabelBinding.h
    bindings/LineEditBinding.h
    bindings/PixmapLabelBinding.h
    bindings/RegisterBinding.h
    bindings/SpinBoxBinding.h
    functions_widgets.h

    uis/Auth.ui
    uis/User.ui
    uis/Users.ui
    uis/about.ui
    uis/combobox_dialog.ui
    uis/doublespinbox_dialog.ui
    uis/menu_corner_widget.ui
    uis/modbus_settings.ui
    uis/scpi_settings.ui
    uis/settings.ui
)

target_include_directories(core-widgets-lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/bindings>
    $<INSTALL_INTERFACE:${CORE_INSTALL_INCLUDEDIR}/core-widgets-lib/bindings>
)

if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(core-widgets-lib PRIVATE Qt6::Core5Compat) #QTextCodec
endif()
