#include "AdvancedClickablePixmapLabelBinding.h"

#include "BindingManager.h"

AdvancedClickablePixmapLabelBinding::AdvancedClickablePixmapLabelBinding(QLabel* label,
                                                                         std::shared_ptr<BindingManager> bindingManager,
                                                                         const QString& registerName,
                                                                         int bitPosition,
                                                                         const QPixmap& pixmapOn,
                                                                         const QPixmap& pixmapOff,
                                                                         BindingDirection direction,
                                                                         BytesFormat modbus_format,
                                                                         QObject* parent)
    : ClickablePixmapLabelBinding(label, registerName, bitPosition, pixmapOn, pixmapOff, direction, modbus_format, parent)
    , m_bindingManager(bindingManager)
{
}

void AdvancedClickablePixmapLabelBinding::invertBit()
{
    if (!m_useBit || m_bitPosition < 0 || m_bitPosition > 15 || !m_bindingManager) {
        //qWarning() << "AdvancedClickablePixmapLabelBinding: Cannot invert bit - invalid parameters";
        return;
    }

    // Получаем текущее значение из кэша менеджера
    QVector<quint16> currentData = m_bindingManager->getRegisterCache(registerName());
    if (currentData.isEmpty()) {
        //qWarning() << "AdvancedClickablePixmapLabelBinding: No cached data for register" << registerName();
        return;
    }

    // Инвертируем бит
    quint16 currentValue = currentData.first();
    quint16 newValue = currentValue ^ (1 << m_bitPosition);

    // qCDebug(logDebug()) << "AdvancedClickablePixmapLabelBinding: Inverting bit" << m_bitPosition
    //                     << "in register" << registerName()
    //                     << "from" << currentValue << "to" << newValue;

    // Отправляем новое значение
    QVector<quint16> newData(1, newValue);
    emit registerValueRequested(registerName(), newData);
}
