#ifndef CLICKABLEPIXMAPLABELBINDING_H
#define CLICKABLEPIXMAPLABELBINDING_H

#include <QMouseEvent>

#include "PixmapLabelBinding.h"

class CORE_MODBUS_LIB_EXPORT ClickablePixmapLabelBinding : public PixmapLabelBinding {
    Q_OBJECT
public:
    ClickablePixmapLabelBinding(QLabel* label,
                                const QString& registerName,
                                int bitPosition,
                                const QPixmap& pixmapOn,
                                const QPixmap& pixmapOff,
                                BindingDirection direction = ReadWrite,
                                BytesFormat modbus_format = BigEndian,
                                QObject* parent = nullptr);

    ClickablePixmapLabelBinding(QLabel* label,
                                const QString& registerName,
                                const QMap<quint16, QPixmap>& statePixmaps,
                                BindingDirection direction = ReadWrite,
                                BytesFormat modbus_format = BigEndian,
                                QObject* parent = nullptr);

    // Включает/выключает возможность клика
    void setClickable(bool clickable);
    bool isClickable() const { return m_clickable; }

    // Устанавливает задержку между кликами (для защиты от случайных многократных нажатий)
    void setClickDelay(int delayMs) { m_clickDelay = delayMs; }
    int clickDelay() const { return m_clickDelay; }

protected:
    // Фильтр событий для обработки кликов
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void processClick();

protected:
    bool m_clickable;
    int m_clickDelay;
    QTimer* m_clickTimer;
    bool m_processingClick;

    void setupClickHandler();
    virtual void invertBit();
};
#endif // CLICKABLEPIXMAPLABELBINDING_H
