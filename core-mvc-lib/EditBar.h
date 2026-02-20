#ifndef EDITBAR_H
#define EDITBAR_H

#include <QToolBar>
#include <QToolButton>

#include "core-mvc-lib-global.h"

class CORE_MVC_LIB_EXPORT EditBar : public QToolBar {
    Q_OBJECT
public:
    EditBar(QWidget *parent = nullptr);
    virtual ~EditBar();

    void set_visible(std::map<QString, bool> edit_tb_enable_m);
    void disable_separators();

    QToolButton *tb_add = new QToolButton(this);
    QToolButton *tb_add_before = new QToolButton(this);
    QToolButton *tb_add_after = new QToolButton(this);
    QToolButton *tb_copy = new QToolButton(this);
    QToolButton *tb_cut = new QToolButton(this);
    QToolButton *tb_paste = new QToolButton(this);
    QToolButton *tb_paste_before = new QToolButton(this);
    QToolButton *tb_paste_after = new QToolButton(this);
    QToolButton *tb_delete = new QToolButton(this);
    QToolButton *tb_save = new QToolButton(this);
    QToolButton *tb_export = new QToolButton(this);
    QToolButton *tb_cancel = new QToolButton(this);
    QToolButton *tb_ok = new QToolButton(this);

signals:

public slots:

private slots:

private:
    QVector<QToolButton*> buttons;
    QVector<QIcon> button_icons;
    QVector<QString> button_texts;

    QVector<int> button_enables;
    QVector<int> button_separators;

    QVector<QAction*> widget_actions;
};

#endif // EDITBAR_H
