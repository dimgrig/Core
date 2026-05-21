#include "EditBar.h"

#include "IconManager.h"
#include "templates.h"
#include "User.h"

EditBar::EditBar(QWidget *parent) :
    QToolBar(parent)
{
    static const char* edit_tb_names[] = {
        "tb_edit_add",
        "tb_edit_add_before",
        "tb_edit_add_after",
        "tb_edit_copy",
        "tb_edit_cut",
        "tb_edit_paste",
        "tb_edit_paste_before",
        "tb_edit_paste_after",
        "tb_edit_delete",
        "tb_edit_save",
        "tb_edit_print",
        "tb_edit_export",
        "tb_edit_cancel",
        "tb_edit_ok",
        "tb_edit_custom1",
        "tb_edit_custom2",
        "tb_edit_custom3"
    };

    static const char* tooltips_v[] = {
        "Добавить",
        "Добавить до",
        "Добавить после",
        "Копировать",
        "Вырезать",
        "Вставить (заменить)",
        "Вставить до",
        "Вставить после",
        "Удалить",
        "Сохранить",
        "Печать",
        "Экспорт",
        "Отмена",
        "Ок",
        "",
        "",
        ""
    };

    static const char* edit_action_names[] = {
        "edit_add",
        "edit_add_before",
        "edit_add_after",
        "edit_add_separator",
        "edit_copy",
        "edit_cut",
        "edit_paste",
        "edit_paste_before",
        "edit_paste_after",
        "edit_copy_paste_separator",
        "edit_delete",
        "edit_delete_separator",
        "edit_save",
        "edit_save_separator",
        "edit_print",
        "edit_export",
        "edit_cancel",
        "edit_ok",
        "edit_ok_separator",
        "edit_custom1",
        "edit_custom2",
        "edit_custom3"
    };

    this->setObjectName("Панель инструментов");

    this->setAttribute(Qt::WA_AlwaysShowToolTips);

    this->setMovable(false);
    this->setFloatable(false);
    this->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonIconOnly);
    this->setIconSize(ICON_SIZE_24);
    this->setObjectName("EditBar");
    this->setAccessibleName("EditBar");
    //this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    this->setOrientation(Qt::Horizontal);

    button_icons.push_back(ICON_ADD);
    button_icons.push_back(ICON_ADD_BEFORE);
    button_icons.push_back(ICON_ADD_AFTER);
    button_icons.push_back(ICON_COPY);
    button_icons.push_back(ICON_CUT);
    button_icons.push_back(ICON_PASTE);
    button_icons.push_back(ICON_INSERT_BEFORE);
    button_icons.push_back(ICON_INSERT_AFTER);
    button_icons.push_back(ICON_DELETE);
    button_icons.push_back(ICON_SAVE);
    button_icons.push_back(ICON_PRINT);
    button_icons.push_back(ICON_EXPORT);
    button_icons.push_back(ICON_CANCEL);
    button_icons.push_back(ICON_OK);
    button_icons.push_back(QIcon());
    button_icons.push_back(QIcon());
    button_icons.push_back(QIcon());

    for (const auto &icon : button_icons) {
        Q_UNUSED(icon);
        button_texts.push_back("");
    }

    buttons.push_back(tb_add);
    buttons.push_back(tb_add_before);
    buttons.push_back(tb_add_after);
    buttons.push_back(tb_copy);
    buttons.push_back(tb_cut);
    buttons.push_back(tb_paste);
    buttons.push_back(tb_paste_before);
    buttons.push_back(tb_paste_after);
    buttons.push_back(tb_delete);
    buttons.push_back(tb_save);
    buttons.push_back(tb_print);
    buttons.push_back(tb_export);
    buttons.push_back(tb_cancel);
    buttons.push_back(tb_ok);
    buttons.push_back(tb_custom1);
    buttons.push_back(tb_custom2);
    buttons.push_back(tb_custom3);

    // после инициализации buttons вектора
    m_buttonMap = {
        {Button::Add,         tb_add},
        {Button::AddBefore,   tb_add_before},
        {Button::AddAfter,    tb_add_after},
        {Button::Copy,        tb_copy},
        {Button::Cut,         tb_cut},
        {Button::Paste,       tb_paste},
        {Button::PasteBefore, tb_paste_before},
        {Button::PasteAfter,  tb_paste_after},
        {Button::Delete,      tb_delete},
        {Button::Save,        tb_save},
        {Button::Print,       tb_print},
        {Button::Export,      tb_export},
        {Button::Cancel,      tb_cancel},
        {Button::Ok,          tb_ok},
        {Button::Custom1,     tb_custom1},
        {Button::Custom2,     tb_custom2},
        {Button::Custom3,     tb_custom3}
    };

    for (auto &[btn, icon, text] : qzip(buttons, button_icons, button_texts)) {
        btn->setIcon(icon);
        btn->setText(text);
    }

    for (auto &btn : buttons) {
        btn->setIconSize(ICON_SIZE_24);
        btn->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonIconOnly);
        //btn->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        btn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    }

    // widget_actions.append({
    //     this->addWidget(tb_add),
    //     this->addWidget(tb_add_before),
    //     this->addWidget(tb_add_after),
    //     this->addSeparator(),
    //     this->addWidget(tb_copy),
    //     this->addWidget(tb_cut),
    //     this->addWidget(tb_paste),
    //     this->addWidget(tb_paste_before),
    //     this->addWidget(tb_paste_after),
    //     this->addSeparator(),
    //     this->addWidget(tb_delete),
    //     this->addSeparator(),
    //     this->addWidget(tb_save),
    //     this->addSeparator(),
    //     this->addWidget(tb_print),
    //     this->addWidget(tb_export),
    //     this->addWidget(tb_cancel),
    //     this->addWidget(tb_ok),
    //     this->addSeparator(),
    //     this->addWidget(tb_custom1),
    //     this->addWidget(tb_custom2),
    //     this->addWidget(tb_custom3),
    // });

    // Добавляем виджеты и запоминаем действия
    auto addButton = [this](Button button, QToolButton* btn) -> QAction* {
        QAction* act = this->addWidget(btn);
        m_actionMap[button] = act;
        return act;
    };

    widget_actions.append(addButton(Button::Add, tb_add));
    widget_actions.append(addButton(Button::AddBefore, tb_add_before));
    widget_actions.append(addButton(Button::AddAfter, tb_add_after));
    widget_actions.append(this->addSeparator());
    widget_actions.append(addButton(Button::Copy, tb_copy));
    widget_actions.append(addButton(Button::Cut, tb_cut));
    widget_actions.append(addButton(Button::Paste, tb_paste));
    widget_actions.append(addButton(Button::PasteBefore, tb_paste_before));
    widget_actions.append(addButton(Button::PasteAfter, tb_paste_after));
    widget_actions.append(this->addSeparator());
    widget_actions.append(addButton(Button::Delete, tb_delete));
    widget_actions.append(this->addSeparator());
    widget_actions.append(addButton(Button::Save, tb_save));
    widget_actions.append(this->addSeparator());
    widget_actions.append(addButton(Button::Print, tb_print));
    widget_actions.append(addButton(Button::Export, tb_export));
    widget_actions.append(addButton(Button::Cancel, tb_cancel));
    widget_actions.append(addButton(Button::Ok, tb_ok));
    widget_actions.append(this->addSeparator());
    widget_actions.append(addButton(Button::Custom1, tb_custom1));
    widget_actions.append(addButton(Button::Custom2, tb_custom2));
    widget_actions.append(addButton(Button::Custom3, tb_custom3));

    // for (auto &[name, tb] : qzip(edit_tb_names, buttons)) {
    //     tb->setObjectName(name.c_str());
    // }

    // for (auto &[tooltip, tb] : qzip(tooltips_v, buttons)) {
    //     tb->setToolTip(tooltip.c_str());
    // }

    // for (auto &[name, action] : qzip(edit_action_names, widget_actions)) {
    //     action->setObjectName(name.c_str());
    // }

    if (buttons.size() == (sizeof(edit_tb_names) / sizeof(edit_tb_names[0]))) {
        for (int i = 0; i < buttons.size(); ++i) {
            buttons[i]->setObjectName(edit_tb_names[i]);
        }
    } else {
        qCDebug(logWarning) << "EditBar vectors size mismath";
    }
    if (buttons.size() == (sizeof(tooltips_v) / sizeof(tooltips_v[0]))) {
        for (int i = 0; i < buttons.size(); ++i) {
            buttons[i]->setToolTip(tooltips_v[i]);
        }
    } else {
        qCDebug(logWarning) << "EditBar vectors size mismath";
    }
    if (widget_actions.size() == (sizeof(edit_action_names) / sizeof(edit_action_names[0]))) {
        for (int i = 0; i < widget_actions.size(); ++i) {
            widget_actions[i]->setObjectName(edit_action_names[i]);
        }
    } else {
        qCDebug(logWarning) << "EditBar vectors size mismath";
    }
}

EditBar::~EditBar() {

}

// void EditBar::set_visible(std::map<QString, bool> edit_tb_enable_m) {
//     for (auto &[name, visible] : edit_tb_enable_m) {
//         int i = 0;
//         for (auto &tb : buttons) {
//             if (tb->objectName() == name) {
//                 tb->setVisible(visible);
//                 if (!visible) {
//                     QString action_name = name;
//                     action_name.remove(0, 3);
//                     for (auto &action : widget_actions) {
//                         if (action->objectName() == action_name) {
//                             this->removeAction(action);
//                             break;
//                         }
//                     }
//                 }
//                 break;
//             }
//             ++i;
//         }
//     }
// }

void EditBar::setButtonVisible(Button button, bool visible) {
    QToolButton *btn = m_buttonMap.value(button);
    QAction *action = m_actionMap.value(button);
    if (btn) {
        btn->setVisible(visible);
    }
    if (action) {
        action->setVisible(visible);
    }
}

void EditBar::setButtonIcon(Button button, const QIcon &icon) {
    if (auto *btn = m_buttonMap.value(button)) {
        btn->setIcon(icon);
    }
}

void EditBar::setButtonToolTip(Button button, const QString &tooltip) {
    if (auto *btn = m_buttonMap.value(button)) {
        btn->setToolTip(tooltip);
    }
}

void EditBar::disable_separators() {
    for (auto &action : widget_actions) {
        if (action->isSeparator()) {
            this->removeAction(action);
        }
    }
}
