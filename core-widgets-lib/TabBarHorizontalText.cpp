#include "TabBarHorizontalText.h"

#include <QTextDocument>
#include <QTextLayout>

TabBarHorizontalText::TabBarHorizontalText(QWidget* parent)
    : QTabBar(parent)
    , m_sizeCache(100)
{
    setIconSize(QSize(18, 18));
    ensurePolished();
}

TabBarHorizontalText::~TabBarHorizontalText() {
    m_sizeCache.clear();
}

void TabBarHorizontalText::clearSizeCache() {
    m_sizeCache.clear();
}

void TabBarHorizontalText::setMaxTabWidth(int width) {
    if (m_maxTabWidth != width) {
        m_maxTabWidth = width;
        clearSizeCache();
        updateGeometry();
    }
}

void TabBarHorizontalText::setPadding(int padding) {
    if (m_padding != padding) {
        m_padding = padding;
        clearSizeCache();
        updateGeometry();
    }
}

void TabBarHorizontalText::setIconTextSpacing(int spacing) {
    if (m_iconTextSpacing != spacing) {
        m_iconTextSpacing = spacing;
        clearSizeCache();
        updateGeometry();
    }
}

void TabBarHorizontalText::setMinTabHeight(int height) {
    if (m_minTabHeight != height) {
        m_minTabHeight = height;
        clearSizeCache();
        updateGeometry();
    }
}

void TabBarHorizontalText::setWordWrap(bool wrap) {
    if (m_wordWrap != wrap) {
        m_wordWrap = wrap;
        clearSizeCache();
        updateGeometry();
        update();
    }
}

void TabBarHorizontalText::setElideMode(Qt::TextElideMode mode) {
    if (m_elideMode != mode) {
        m_elideMode = mode;
        clearSizeCache();
        update();
    }
}

QStringList TabBarHorizontalText::splitTextIntoLines(const QString &text, int maxWidth) const {
    QStringList lines;

    if (!m_wordWrap || maxWidth <= 0) {
        // Если перенос слов отключен, просто разбиваем по символу новой строки
        lines = text.split('\n', Qt::SkipEmptyParts);
        if (lines.isEmpty()) lines << ""; // Одна пустая строка
        return lines;
    }

    // Используем QTextLayout для правильного переноса слов
    QTextLayout textLayout(text, font());
    textLayout.beginLayout();

    QTextLine line;
    int y = 0;

    while ((line = textLayout.createLine()).isValid()) {
        line.setLineWidth(maxWidth);
        int nextLineY = y + line.height();

        if (text.length() > 0) {
            lines << text.mid(line.textStart(), line.textLength()).trimmed();
            y = nextLineY;
        }
    }

    textLayout.endLayout();

    if (lines.isEmpty()) {
        lines << "";
    }

    return lines;
}

// TabBarHorizontalText::TabSizeCache TabBarHorizontalText::calculateTabSize(int index) const {
//     TabSizeCache cache;

//     if (index < 0 || index >= count()) {
//         cache.size = QSize(m_maxTabWidth, m_minTabHeight);
//         return cache;
//     }

//     QString text = tabText(index);
//     QIcon icon = tabIcon(index);
//     QFontMetrics fm = fontMetrics();

//     // Учитываем отступы
//     int availableWidth = m_maxTabWidth - 2 * m_padding;
//     if (!icon.isNull()) {
//         availableWidth -= (iconSize().width() + m_iconTextSpacing);
//     }

//     // Разбиваем текст на строки
//     cache.lines = splitTextIntoLines(text, availableWidth);

//     // Вычисляем максимальную ширину строки
//     int maxLineWidth = 0;
//     for (const QString &line : cache.lines) {
//         int lineWidth = fm.horizontalAdvance(line);
//         maxLineWidth = qMax(maxLineWidth, lineWidth);
//     }

//     // Ограничиваем ширину
//     maxLineWidth = qMin(maxLineWidth, availableWidth);

//     // Вычисляем общую ширину
//     int totalWidth = maxLineWidth + 2 * m_padding;
//     if (!icon.isNull()) {
//         totalWidth += iconSize().width() + m_iconTextSpacing;
//     }
//     totalWidth = qMin(totalWidth, m_maxTabWidth);

//     // Вычисляем высоту текста
//     int lineHeight = fm.height();
//     int textHeight = cache.lines.size() * (lineHeight + m_lineSpacing);
//     if (!cache.lines.isEmpty()) {
//         textHeight -= m_lineSpacing; // Убираем последний межстрочный интервал
//     }

//     // Общая высота
//     int totalHeight = textHeight + 2 * m_padding;
//     if (!icon.isNull()) {
//         totalHeight += iconSize().height() + m_iconTextSpacing;
//     }
//     totalHeight = qMax(totalHeight, m_minTabHeight);

//     cache.size = QSize(totalWidth, totalHeight);

//     // Сохраняем прямоугольник для текста
//     int textTop = m_padding;
//     if (!icon.isNull()) {
//         textTop += iconSize().height() + m_iconTextSpacing;
//     }
//     cache.textRect = QRectF(m_padding, textTop,
//                             totalWidth - 2 * m_padding,
//                             totalHeight - textTop - m_padding);

//     return cache;
// }

QSize TabBarHorizontalText::tabSizeHint(int index) const {
    if (index < 0 || index >= count())
        return QSize(m_maxTabWidth, m_minTabHeight);

    QString text = tabText(index);
    QIcon icon = tabIcon(index);
    QFontMetrics fm(font());

    // Разбиваем текст на строки
    QStringList lines = text.split('\n', Qt::SkipEmptyParts);
    if (lines.isEmpty()) lines << "";

    // Находим максимальную ширину строки
    int maxLineWidth = 0;
    for (const QString &line : lines) {
        int lineWidth = fm.horizontalAdvance(line);
        maxLineWidth = qMax(maxLineWidth, lineWidth);
    }

    // Учитываем отступы и иконку
    int totalWidth = maxLineWidth + 2 * m_padding;
    if (!icon.isNull()) {
        totalWidth += iconSize().width() + m_iconTextSpacing;
    }
    totalWidth = qMin(totalWidth, m_maxTabWidth);

    // Рассчитываем высоту
    int lineHeight = fm.height();
    int textHeight = lines.size() * (lineHeight + m_lineSpacing);
    if (!lines.isEmpty()) textHeight -= m_lineSpacing;

    int totalHeight = textHeight + 2 * m_padding;
    if (!icon.isNull()) {
        totalHeight += iconSize().height() + m_iconTextSpacing;
    }
    totalHeight = qMax(totalHeight, m_minTabHeight);

    return QSize(totalWidth, totalHeight);
}

void TabBarHorizontalText::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QStylePainter p(this);
    QStyleOptionTabV3 opt;
    QFontMetrics fm = p.fontMetrics();

    for (int index = 0; index < count(); index++) {
        initStyleOption(&opt, index);

        QIcon originalIcon = opt.icon;
        QString originalText = opt.text;
        opt.icon = QIcon();
        opt.text = QString();

        // Рисуем фон таба
        p.drawControl(QStyle::CE_TabBarTab, opt);

        // Получаем прямоугольник таба
        QRect tabRect = this->tabRect(index);

        // Область для содержимого (с отступами)
        QRect contentRect = tabRect.adjusted(m_padding, m_padding, -m_padding, -m_padding);

        // Разбиваем текст на строки
        QStringList lines = originalText.split('\n', Qt::SkipEmptyParts);
        if (lines.isEmpty()) lines << "";

        if (!originalIcon.isNull()) {
            // Рисуем иконку
            QSize iconSize = this->iconSize();
            int iconX = contentRect.left() + (contentRect.width() - iconSize.width()) / 2;
            int iconY = contentRect.top();
            QRect iconRect(iconX, iconY, iconSize.width(), iconSize.height());
            originalIcon.paint(&p, iconRect, Qt::AlignCenter);

            // Область для текста под иконкой
            QRect textRect = contentRect;
            textRect.setTop(iconRect.bottom() + m_iconTextSpacing);

            // Рассчитываем высоту текста
            int lineHeight = fm.height();
            int totalTextHeight = lines.size() * (lineHeight + m_lineSpacing);
            if (!lines.isEmpty()) totalTextHeight -= m_lineSpacing;

            // Центрируем текст вертикально
            int textY = textRect.top();
            if (totalTextHeight < textRect.height()) {
                textY += (textRect.height() - totalTextHeight) / 2;
            }

            // Рисуем каждую строку
            for (int i = 0; i < lines.size(); i++) {
                QString line = lines[i];
                if (fm.horizontalAdvance(line) > textRect.width()) {
                    line = fm.elidedText(line, Qt::ElideRight, textRect.width());
                }

                QRect lineRect(textRect.left(),
                               textY + i * (lineHeight + m_lineSpacing),
                               textRect.width(),
                               lineHeight);

                p.drawText(lineRect, Qt::AlignHCenter | Qt::AlignTop, line);
            }
        } else {
            // Только текст
            int lineHeight = fm.height();
            int totalTextHeight = lines.size() * (lineHeight + m_lineSpacing);
            if (!lines.isEmpty()) totalTextHeight -= m_lineSpacing;

            // Центрируем текст вертикально
            int textY = contentRect.top();
            if (totalTextHeight < contentRect.height()) {
                textY += (contentRect.height() - totalTextHeight) / 2;
            }

            // Рисуем каждую строку
            for (int i = 0; i < lines.size(); i++) {
                QString line = lines[i];
                if (fm.horizontalAdvance(line) > contentRect.width()) {
                    line = fm.elidedText(line, Qt::ElideRight, contentRect.width());
                }

                QRect lineRect(contentRect.left(),
                               textY + i * (lineHeight + m_lineSpacing),
                               contentRect.width(),
                               lineHeight);

                p.drawText(lineRect, Qt::AlignHCenter | Qt::AlignTop, line);
            }
        }
    }
}

void TabBarHorizontalText::tabInserted(int index) {
    clearSizeCache();
    QTabBar::tabInserted(index);
}

void TabBarHorizontalText::tabRemoved(int index) {
    clearSizeCache();
    QTabBar::tabRemoved(index);
}

void TabBarHorizontalText::changeEvent(QEvent *event) {
    if (event->type() == QEvent::FontChange ||
        event->type() == QEvent::StyleChange ||
        event->type() == QEvent::LanguageChange) {
        clearSizeCache();
    }
    QTabBar::changeEvent(event);
}
