#include "BindingFactory.h"

////#include "icons.h"

#define __LED_RED_ON QPixmap(":/icons/led_red_on.svg")
#define __LED_RED_OFF QPixmap(":/icons/led_red_off.svg")
#define __LED_GREEN_ON QPixmap(":/icons/led_green_on.svg")
#define __LED_GREEN_OFF QPixmap(":/icons/led_green_off.svg")

#define __K_OFF QPixmap(":/icons/K_OFF.svg")
#define __K_ON QPixmap(":/icons/K_ON.svg")
#define __N_OFF QPixmap(":/icons/N_OFF.svg")
#define __N_ON QPixmap(":/icons/N_ON.svg")


BindingFactory::BindingFactory(std::shared_ptr<BindingManager> bindingManager) : m_bindingManager(bindingManager) {}

// Создает кликабельный индикатор с стандартными изображениями
std::shared_ptr<AdvancedClickablePixmapLabelBinding> BindingFactory::createClickableGreenLedIndicator(QLabel* label,
                                                                 const QString& registerName,
                                                                 int bitPosition,
                                                                 const QSize& size,
                                                                 RegisterBinding::BindingDirection direction,
                                                                 ModbusFormat modbus_format) {
    return std::make_shared<AdvancedClickablePixmapLabelBinding>(
        label,
        m_bindingManager,
        registerName,
        bitPosition,
        __LED_GREEN_ON,
        __LED_GREEN_OFF,
        direction,
        modbus_format
        );
}

std::shared_ptr<AdvancedClickablePixmapLabelBinding> BindingFactory::createClickableRedLedIndicator(QLabel* label,
                                                                                      const QString& registerName,
                                                                                      int bitPosition,
                                                                                      const QSize& size,
                                                                                      RegisterBinding::BindingDirection direction,
                                                                                      ModbusFormat modbus_format) {
    return std::make_shared<AdvancedClickablePixmapLabelBinding>(
        label,
        m_bindingManager,
        registerName,
        bitPosition,
        __LED_RED_ON,
        __LED_RED_OFF,
        direction,
        modbus_format
        );
}

// Создает кликабельный индикатор насоса
std::shared_ptr<AdvancedClickablePixmapLabelBinding> BindingFactory::createClickablePumpIndicator(QLabel* label,
                                                                  const QString& registerName,
                                                                  int bitPosition,
                                                                  const QSize& size,
                                                                  RegisterBinding::BindingDirection direction,
                                                                  ModbusFormat modbus_format) {
    return std::make_shared<AdvancedClickablePixmapLabelBinding>(
        label,
        m_bindingManager,
        registerName,
        bitPosition,
        __N_ON,
        __N_OFF,
        direction,
        modbus_format
        );
}

// Создает кликабельный индикатор вентиля
std::shared_ptr<AdvancedClickablePixmapLabelBinding> BindingFactory::createClickableValveIndicator(QLabel* label,
                                                                   const QString& registerName,
                                                                   int bitPosition,
                                                                   const QSize& size,
                                                                   RegisterBinding::BindingDirection direction,
                                                                   ModbusFormat modbus_format) {
    return std::make_shared<AdvancedClickablePixmapLabelBinding>(
        label,
        m_bindingManager,
        registerName,
        bitPosition,
        __K_ON,
        __K_OFF,
        direction,
        modbus_format
        );
}
