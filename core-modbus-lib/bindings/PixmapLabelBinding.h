#ifndef PIXMAPLABELBINDING_H
#define PIXMAPLABELBINDING_H

#include <QLabel>
#include <QPixmap>
#include <QMap>

#include "RegisterBinding.h"

class CORE_MODBUS_LIB_EXPORT PixmapLabelBinding : public RegisterBinding {
    Q_OBJECT
public:
    // Конструктор для двух состояний (вкл/выкл)
    PixmapLabelBinding(QLabel* label,
                       const QString& registerName,
                       int bitPosition,
                       const QPixmap& pixmapOn,
                       const QPixmap& pixmapOff,
                       BindingDirection direction = ReadOnly,
                       ModbusFormat modbus_format = BigEndian,
                       QObject* parent = nullptr);

    // Конструктор для множества состояний (значение -> pixmap)
    PixmapLabelBinding(QLabel* label,
                       const QString& registerName,
                       const QMap<quint16, QPixmap>& statePixmaps,
                       BindingDirection direction = ReadOnly,
                       ModbusFormat modbus_format = BigEndian,
                       QObject* parent = nullptr);

    // Конструктор для работы с целым значением регистра
    PixmapLabelBinding(QLabel* label,
                       const QString& registerName,
                       const QMap<quint16, QPixmap>& valuePixmaps,
                       bool useBit = false,
                       BindingDirection direction = ReadOnly,
                       ModbusFormat modbus_format = BigEndian,
                       QObject* parent = nullptr);

    void updateFromRegister(const QVector<quint16>& data) override;
    QVector<quint16> getRegisterData() const override;
    bool isValid() const override;

    // Методы для управления изображениями
    void setPixmapForState(quint16 value, const QPixmap& pixmap);
    void setPixmapForBitState(bool state, const QPixmap& pixmap);
    void clearPixmaps();

protected:
    QLabel* m_label;
    int m_bitPosition;
    bool m_useBit;
    QMap<quint16, QPixmap> m_statePixmaps;
    QPixmap m_pixmapOn;
    QPixmap m_pixmapOff;
};


#endif // PIXMAPLABELBINDING_H
