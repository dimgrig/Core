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

    enum class Button {
        Add,
        AddBefore,
        AddAfter,
        Copy,
        Cut,
        Paste,
        PasteBefore,
        PasteAfter,
        Delete,
        Save,
        Print,
        Export,
        Cancel,
        Ok,
        Custom1,
        Custom2,
        Custom3
    };
    Q_ENUM(Button)

    void setButtonVisible(Button button, bool visible);
    void setButtonIcon(Button button, const QIcon &icon);
    void setButtonToolTip(Button button, const QString &tooltip);
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
    QToolButton *tb_print = new QToolButton(this);
    QToolButton *tb_export = new QToolButton(this);
    QToolButton *tb_cancel = new QToolButton(this);
    QToolButton *tb_ok = new QToolButton(this);
    QToolButton *tb_custom1 = new QToolButton(this);
    QToolButton *tb_custom2 = new QToolButton(this);
    QToolButton *tb_custom3 = new QToolButton(this);


signals:

public slots:

private slots:

private:
    QVector<QToolButton*> buttons;
    QVector<QIcon> button_icons;
    QVector<QString> button_texts;

    QVector<QAction*> widget_actions;

    QMap<Button, QAction*> m_actionMap;
    QMap<Button, QToolButton*> m_buttonMap;
};

#endif // EDITBAR_H
