// BindingFactory.h
#ifndef BINDINGFACTORY_H
#define BINDINGFACTORY_H

#include "AdvancedClickablePixmapLabelBinding.h"
#include "BindingManager.h"

class CORE_MODBUS_LIB_EXPORT BindingFactory {
public:
    BindingFactory(std::shared_ptr<BindingManager> bindingManager);

    // Создает кликабельный индикатор с стандартными изображениями
    std::shared_ptr<AdvancedClickablePixmapLabelBinding> createClickableGreenLedIndicator(QLabel* label,
                                                                     const QString& registerName,
                                                                     int bitPosition,
                                                                     const QSize& size = QSize(16, 16),
                                                                     RegisterBinding::BindingDirection direction = RegisterBinding::ReadWrite,
                                                                     BytesFormat modbus_format = BigEndian);
    std::shared_ptr<AdvancedClickablePixmapLabelBinding> createClickableRedLedIndicator(QLabel* label,
                                                                          const QString& registerName,
                                                                          int bitPosition,
                                                                          const QSize& size = QSize(16, 16),
                                                                          RegisterBinding::BindingDirection direction = RegisterBinding::ReadWrite,
                                                                          BytesFormat modbus_format = BigEndian);

    // Создает кликабельный индикатор насоса
    std::shared_ptr<AdvancedClickablePixmapLabelBinding> createClickablePumpIndicator(QLabel* label,
                                                                      const QString& registerName,
                                                                      int bitPosition,
                                                                      const QSize& size = QSize(32, 32),
                                                                      RegisterBinding::BindingDirection direction = RegisterBinding::ReadWrite,
                                                                      BytesFormat modbus_format = BigEndian);

    // Создает кликабельный индикатор вентиля
    std::shared_ptr<AdvancedClickablePixmapLabelBinding> createClickableValveIndicator(QLabel* label,
                                                                       const QString& registerName,
                                                                       int bitPosition,
                                                                       const QSize& size = QSize(64, 32),
                                                                       RegisterBinding::BindingDirection direction = RegisterBinding::ReadWrite,
                                                                       BytesFormat modbus_format = BigEndian);

private:
    std::shared_ptr<BindingManager> m_bindingManager;
};

#endif // BINDINGFACTORY_H
