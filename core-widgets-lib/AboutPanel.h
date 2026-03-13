#ifndef ABOUTPANEL_H
#define ABOUTPANEL_H

#include <QWidget>

#include "core-widgets-lib-global.h"

namespace Ui {
    class fabout;
}

class CORE_WIDGETS_LIB_EXPORT AboutPanel : public QWidget {
Q_OBJECT
public:
    explicit AboutPanel(QWidget *parent = nullptr,
                       QPixmap pixmap = QPixmap(),
                       QString organizationName = "",
                       QString applicationName = "",
                       QString version= "");
    ~AboutPanel();
public slots:
    void read_settings();
private slots:

private:
    void showEvent(QShowEvent *event);

    Ui::fabout *ui;

    QPixmap _pixmap;
    QString _organizationName;
    QString _applicationName;
    QString _version;
};


#endif // ABOUTPANEL_H
