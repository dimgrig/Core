#include "ClickablePixmapLabelBinding.h"

#include <QTimer>
#include <QMouseEvent>
#include <QApplication>

#include "Logger.h"

ClickablePixmapLabelBinding::ClickablePixmapLabelBinding(QLabel* label,
                                                         const QString& registerName,
                                                         int bitPosition,
                                                         const QPixmap& pixmapOn,
                                                         const QPixmap& pixmapOff,
                                                         BindingDirection direction,
                                                         ModbusFormat modbus_format,
                                                         QObject* parent)
    : PixmapLabelBinding(label, registerName, bitPosition, pixmapOn, pixmapOff, direction, modbus_format, parent)
    , m_clickable(true)
    , m_clickDelay(300) // 300ms задержка по умолчанию
    , m_processingClick(false)
{
    m_clickTimer = new QTimer(this);
    m_clickTimer->setSingleShot(true);
    connect(m_clickTimer, &QTimer::timeout, this, &ClickablePixmapLabelBinding::processClick);

    setupClickHandler();
}

ClickablePixmapLabelBinding::ClickablePixmapLabelBinding(QLabel* label,
                                                         const QString& registerName,
                                                         const QMap<quint16, QPixmap>& statePixmaps,
                                                         BindingDirection direction,
                                                         ModbusFormat modbus_format,
                                                         QObject* parent)
    : PixmapLabelBinding(label, registerName, statePixmaps, direction, modbus_format, parent)
    , m_clickable(true)
    , m_clickDelay(300)
    , m_processingClick(false)
{
    m_clickTimer = new QTimer(this);
    m_clickTimer->setSingleShot(true);
    connect(m_clickTimer, &QTimer::timeout, this, &ClickablePixmapLabelBinding::processClick);

    setupClickHandler();
}

void ClickablePixmapLabelBinding::setupClickHandler()
{
    if (m_label) {
        // Включаем отслеживание мыши
        m_label->setMouseTracking(true);
        m_label->installEventFilter(this);

        // Меняем курсор на указатель, чтобы показать, что элемент кликабельный
        if (m_clickable && direction() != ReadOnly) {
            m_label->setCursor(Qt::PointingHandCursor);
        }
    }
}

void ClickablePixmapLabelBinding::setClickable(bool clickable)
{
    m_clickable = clickable;

    if (m_label) {
        if (clickable && direction() != ReadOnly) {
            m_label->setCursor(Qt::PointingHandCursor);
        } else {
            m_label->setCursor(Qt::ArrowCursor);
        }
    }
}

bool ClickablePixmapLabelBinding::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == m_label && m_clickable && direction() != ReadOnly) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                // Запускаем таймер для обработки клика (с задержкой)
                if (!m_processingClick && !m_clickTimer->isActive()) {
                    m_clickTimer->start(m_clickDelay);
                }
                return true;
            }
        }
        else if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                // Если таймер еще активен, отменяем его (пользователь отпустил кнопку слишком быстро)
                if (m_clickTimer->isActive()) {
                    m_clickTimer->stop();
                }
                return true;
            }
        }
    }

    return PixmapLabelBinding::eventFilter(obj, event);
}

void ClickablePixmapLabelBinding::processClick()
{
    if (m_processingClick) return;

    m_processingClick = true;

    // Визуальная обратная связь - временно меняем курсор
    if (m_label) {
        m_label->setCursor(Qt::WaitCursor);
        QApplication::processEvents(); // Обрабатываем события для обновления UI
    }

    // Инвертируем бит
    invertBit();

    // Восстанавливаем курсор
    if (m_label) {
        m_label->setCursor(Qt::PointingHandCursor);
    }

    m_processingClick = false;
}

void ClickablePixmapLabelBinding::invertBit()
{
    if (!m_useBit || m_bitPosition < 0 || m_bitPosition > 15) {
        qWarning() << "ClickablePixmapLabelBinding: Cannot invert bit - invalid bit position";
        return;
    }

    // Получаем текущее значение регистра из кэша (если доступно)
    // В реальной реализации вам нужно получить текущее значение из BindingManager
    // Для примера будем считать, что у нас есть доступ к текущему значению

    // В реальном коде это должно выглядеть так:
    // QVector<quint16> currentData = m_bindingManager->getRegisterCache(registerName());
    // if (currentData.isEmpty()) return;

    // Для демонстрации создадим фиктивное текущее значение
    // В реальном проекте замените это на получение из кэша
    QVector<quint16> currentData(1, 0);

    // Инвертируем бит
    quint16 currentValue = currentData.first();
    quint16 newValue = currentValue ^ (1 << m_bitPosition);

    // qCDebug(logDebug()) << "ClickablePixmapLabelBinding: Inverting bit" << m_bitPosition
    //                     << "in register" << registerName()
    //                     << "from" << currentValue << "to" << newValue;

    // Отправляем новое значение
    QVector<quint16> newData(1, newValue);
    emit registerValueRequested(registerName(), newData);

    // Визуальная обратная связь - можно добавить анимацию или временное изменение
    if (m_label) {
        // Временно увеличиваем размер для эффекта нажатия
        QSize originalSize = m_label->size();
        m_label->setFixedSize(originalSize * 1.1);

        QTimer::singleShot(100, [this, originalSize]() {
            if (m_label) {
                m_label->setFixedSize(originalSize);
            }
        });
    }
}
