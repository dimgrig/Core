#ifndef TABBARHORIZONTALTEXT_H
#define TABBARHORIZONTALTEXT_H

#include <QTabBar>
#include <QStylePainter>
#include <QStyleOptionTabV3>
#include <QPainter>
#include <QIcon>
#include <QString>
#include <QFontMetrics>
#include <QTextOption>
#include <QCache>

#include "core-widgets-lib-global.h"

class CORE_WIDGETS_LIB_EXPORT TabBarHorizontalText : public QTabBar
{
    Q_OBJECT
    Q_PROPERTY(int maxTabWidth READ maxTabWidth WRITE setMaxTabWidth)
    Q_PROPERTY(int padding READ padding WRITE setPadding)
    Q_PROPERTY(int iconTextSpacing READ iconTextSpacing WRITE setIconTextSpacing)
    Q_PROPERTY(int minTabHeight READ minTabHeight WRITE setMinTabHeight)
    Q_PROPERTY(bool wordWrap READ wordWrap WRITE setWordWrap)
    Q_PROPERTY(Qt::TextElideMode elideMode READ elideMode WRITE setElideMode)

public:
    explicit TabBarHorizontalText(QWidget* parent = nullptr);
    ~TabBarHorizontalText();

    QSize tabSizeHint(int index) const override;
    void paintEvent(QPaintEvent *event) override;

    int maxTabWidth() const { return m_maxTabWidth; }
    void setMaxTabWidth(int width);

    int padding() const { return m_padding; }
    void setPadding(int padding);

    int iconTextSpacing() const { return m_iconTextSpacing; }
    void setIconTextSpacing(int spacing);

    int minTabHeight() const { return m_minTabHeight; }
    void setMinTabHeight(int height);

    bool wordWrap() const { return m_wordWrap; }
    void setWordWrap(bool wrap);

    Qt::TextElideMode elideMode() const { return m_elideMode; }
    void setElideMode(Qt::TextElideMode mode);

protected:
    void tabInserted(int index) override;
    void tabRemoved(int index) override;
    void changeEvent(QEvent *event) override;

private:
    struct TabSizeCache {
        QSize size;
        QRectF textRect;
        QStringList lines;
    };

    void clearSizeCache();
    //TabSizeCache calculateTabSize(int index) const;
    QStringList splitTextIntoLines(const QString &text, int maxWidth) const;

    mutable QCache<int, TabSizeCache> m_sizeCache;
    int m_maxTabWidth = 200;
    int m_padding = 8;
    int m_iconTextSpacing = 4;
    int m_minTabHeight = 40;
    bool m_wordWrap = true;
    Qt::TextElideMode m_elideMode = Qt::ElideRight;
    int m_lineSpacing = 2;
};

#endif // TABBARHORIZONTALTEXT_H
