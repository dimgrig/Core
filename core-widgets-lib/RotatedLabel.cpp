#include "RotatedLabel.h"
#include <QPainter>
#include <QFontMetrics>

RotatedLabel::RotatedLabel(QWidget *parent)
    : QLabel(parent)
{
    // Никаких дополнительных настроек
}

void RotatedLabel::setRotation(int degrees) {
    if (_rotation != degrees) {
        _rotation = degrees;
        update();  // перерисовка
    }
}

int RotatedLabel::calcOptimalFontSize() const {
    if (text().isEmpty()) return 1;

    QFont f = font();
    f.setPixelSize(1);
    QFontMetrics fm(f);
    int textWidth = fm.horizontalAdvance(text());
    int textHeight = fm.height();

    // Доступное пространство после поворота
    int w = width();
    int h = height();
    int availableForTextWidth, availableForTextHeight;
    if (_rotation == 90 || _rotation == 270) {
        availableForTextWidth = h;   // ширина текста = высота виджета
        availableForTextHeight = w;  // высота строки = ширина виджета
    } else {
        availableForTextWidth = w;
        availableForTextHeight = h;
    }

    // Подбираем максимальный размер шрифта, умещающийся в доступные размеры
    int fontSize = 1;
    while (fontSize < 200) {
        f.setPixelSize(fontSize + 1);
        QFontMetrics fm2(f);
        if (fm2.horizontalAdvance(text()) > availableForTextWidth ||
            fm2.height() > availableForTextHeight)
            break;
        ++fontSize;
    }
    return fontSize;
}

void RotatedLabel::paintEvent(QPaintEvent *event) {
    // 1. Временно очищаем текст, чтобы QLabel не нарисовал его горизонтально
    QString savedText = text();
    const_cast<RotatedLabel*>(this)->setText(QString());  // без перерисовки
    // Рисуем фон, рамку и всё остальное средствами QLabel
    QLabel::paintEvent(event);
    // Восстанавливаем текст
    const_cast<RotatedLabel*>(this)->setText(savedText);

    if (savedText.isEmpty()) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 2. Подбираем шрифт
    QFont f = font();
    int optimalSize = calcOptimalFontSize();
    f.setPixelSize(optimalSize);
    painter.setFont(f);
    painter.setPen(palette().color(QPalette::WindowText));

    // 3. Определяем прямоугольник для текста в зависимости от поворота
    QRect textRect;
    if (_rotation == 90 || _rotation == 270) {
        // Меняем местами ширину и высоту: строка будет вдоль высоты виджета
        textRect = QRect(0, 0, height(), width());
    } else {
        textRect = QRect(0, 0, width(), height());
    }

    // 4. Трансформации: поворот вокруг центра виджета
    painter.translate(rect().center());
    painter.rotate(_rotation);
    painter.translate(-textRect.center());

    // 5. Рисуем текст
    painter.drawText(textRect, alignment(), savedText);
}
