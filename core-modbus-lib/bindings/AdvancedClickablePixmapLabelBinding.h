#ifndef ADVANCEDCLICKABLEPIXMAPLABELBINDING_H
#define ADVANCEDCLICKABLEPIXMAPLABELBINDING_H

#include "ClickablePixmapLabelBinding.h"

class BindingManager;

class CORE_MODBUS_LIB_EXPORT AdvancedClickablePixmapLabelBinding : public ClickablePixmapLabelBinding {
    Q_OBJECT
public:
    AdvancedClickablePixmapLabelBinding(QLabel* label,
                                        std::shared_ptr<BindingManager> bindingManager,
                                        const QString& registerName,
                                        int bitPosition,
                                        const QPixmap& pixmapOn,
                                        const QPixmap& pixmapOff,
                                        BindingDirection direction = ReadWrite,
                                        ModbusFormat modbus_format = BigEndian,
                                        QObject* parent = nullptr);

protected:
    void invertBit() override;

private:
    std::shared_ptr<BindingManager> m_bindingManager;
};

#endif // ADVANCEDCLICKABLEPIXMAPLABELBINDING_H
