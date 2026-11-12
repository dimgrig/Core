QT       += core
greaterThan(QT_MAJOR_VERSION, 4): QT += gui widgets svg network serialbus serialport mqtt
greaterThan(QT_MAJOR_VERSION, 5): QT += statemachine

TEMPLATE = lib
DEFINES += CORE_WIDGETS_LIB_LIBRARY
CONFIG += c++17

INCLUDEPATH += "$$_PRO_FILE_PWD_/"

HEADERS += \
    AboutPanel.h \
    Approximation.h \
    AspectRatioPixmapLabel.h \
    Auth.h \
    CalendarWidget.h \
    ComboBoxDialog.h \
    ConnectionButton.h \
    DateTimeRangeWidget.h \
    DetailListWidget.h \
    DoubleSpinBoxDialog.h \
    DynamicListPanel.h \
    DynamicListWidget.h \
    FlowLayout.h \
    FocusWatcher.h \
    IUserRoleManager.h \
    IconManager.h \
    MenuBar.h \
    MenuCornerWidget.h \
    MessageBox.h \
    MockUserRoleManager.h \
    ModbusSettingsUI.h \
    MultiSelectWidget.h \
    RotatedLabel.h \
    RoundedLabel.h \
    SCPISettingsUI.h \
    SpinBoxEditor.h \
    TabBarHorizontalText.h \
    User.h \
    WidgetResizeMixin.h \
    aqp.h \
    bindings/AdvancedClickablePixmapLabelBinding.h \
    bindings/BindingFactory.h \
    bindings/BindingManager.h \
    bindings/BitAwareBindingManager.h \
    bindings/CheckBoxBinding.h \
    bindings/ClickablePixmapLabelBinding.h \
    bindings/ComboBoxBinding.h \
    bindings/DoubleSpinBoxBinding.h \
    bindings/LabelBinding.h \
    bindings/LineEditBinding.h \
    bindings/PixmapLabelBinding.h \
    bindings/RegisterBinding.h \
    bindings/SpinBoxBinding.h \
    core-widgets-lib-global.h \
    icons.h

SOURCES += \
    AboutPanel.cpp \
    Approximation.cpp \
    AspectRatioPixmapLabel.cpp \
    Auth.cpp \
    CalendarWidget.cpp \
    ComboBoxDialog.cpp \
    ConnectionButton.cpp \
    DateTimeRangeWidget.cpp \
    DetailListWidget.cpp \
    DoubleSpinBoxDialog.cpp \
    DynamicListPanel.cpp \
    DynamicListWidget.cpp \
    FlowLayout.cpp \
    FocusWatcher.cpp \
    IUserRoleManager.cpp \
    IconManager.cpp \
    MenuBar.cpp \
    MenuCornerWidget.cpp \
    MessageBox.cpp \
    MockUserRoleManager.cpp \
    ModbusSettingsUI.cpp \
    MultiSelectWidget.cpp \
    RotatedLabel.cpp \
    RoundedLabel.cpp \
    SCPISettingsUI.cpp \
    TabBarHorizontalText.cpp \
    User.cpp \
    WidgetResizeMixin.cpp \
    aqp.cpp \
    bindings/AdvancedClickablePixmapLabelBinding.cpp \
    bindings/BindingFactory.cpp \
    bindings/BindingManager.cpp \
    bindings/BitAwareBindingManager.cpp \
    bindings/CheckBoxBinding.cpp \
    bindings/ClickablePixmapLabelBinding.cpp \
    bindings/ComboBoxBinding.cpp \
    bindings/DoubleSpinBoxBinding.cpp \
    bindings/LabelBinding.cpp \
    bindings/LineEditBinding.cpp \
    bindings/PixmapLabelBinding.cpp \
    bindings/RegisterBinding.cpp \
    bindings/SpinBoxBinding.cpp

FORMS += \
    uis/Auth.ui \
    uis/User.ui \
    uis/Users.ui \
    uis/about.ui \
    uis/combobox_dialog.ui \
    uis/doublespinbox_dialog.ui \
    uis/menu_corner_widget.ui \
    uis/modbus_settings.ui \
    uis/scpi_settings.ui \
    uis/settings.ui

RESOURCES += \
    resources/core_widgets_resources.qrc

CORE_LINK_LIBS = core-lib core-journal-lib core-modbus-lib core-io-lib
include(../core-build.pri)
VERSION = $$CORE_VERSION

# Install layout — паритет с CMake install (см. cmake/CoreLib.cmake).
unix:!macx {
    target.path   = $$CORE_INSTALL_PREFIX/$$CORE_INSTALL_LIBDIR
    headers.files = $$HEADERS
    headers.path  = $$CORE_INSTALL_PREFIX/$$CORE_INSTALL_INCLUDEDIR/$$TARGET
}
win32|win64 {
    headers.path  = $$CORE_APP_LOCAL_DIR/$$CORE_INSTALL_INCLUDEDIR/$$TARGET
    INSTALLS += headers

    contains(CONFIG, staticlib) {
        target_lib.files = $$DESTDIR/lib$${TARGET}.a
        target_lib.path  = $$CORE_APP_LOCAL_DIR/$$CORE_INSTALL_LIBDIR
        INSTALLS += target_lib
    } else {
        target_bin.files = $$DESTDIR/$${TARGET}.dll
        target_bin.path  = $$CORE_APP_LOCAL_DIR/$$CORE_INSTALL_BINDIR
        target_lib.files = $$DESTDIR/lib$${TARGET}.a
        target_lib.path  = $$CORE_APP_LOCAL_DIR/$$CORE_INSTALL_LIBDIR
        INSTALLS += target_bin target_lib
    }
}
headers.files =
for(h, HEADERS) {
    headers.files += $$_PRO_FILE_PWD_/$$h
}
INSTALLS += target headers
