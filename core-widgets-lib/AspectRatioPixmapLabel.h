#ifndef ASPECTRATIOPIXMAPLABEL_H
#define ASPECTRATIOPIXMAPLABEL_H

#include <QIcon>
#include <QLabel>
#include <QPixmap>
#include <QResizeEvent>
#include <QString>

#include "macroses.h"

class AspectRatioPixmapLabel : public QLabel
{
    Q_OBJECT
public:
    explicit AspectRatioPixmapLabel(QWidget *parent = 0);
    virtual int heightForWidth(int width) const;
    virtual QSize sizeHint() const;
    QPixmap scaledPixmap();
public slots:
    void setPixmapFromURL(const QString & s);
    void resizeEvent(QResizeEvent *);
private:
    QString url;
    QPixmap pix;
};

#endif // ASPECTRATIOPIXMAPLABEL_H
