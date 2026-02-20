#include "EditBar.h"

#include "IconManager.h"
#include "templates.h"
#include "User.h"

QVector<std::string> edit_tb_names = {
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
    "tb_edit_export",
    "tb_edit_cancel",
    "tb_edit_ok"
};

QVector<std::string> tooltips_v = {
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
    "Экспорт",
    "Отмена",
    "Ок"
};

QVector<std::string> edit_action_names = {
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
    "edit_export",
    "edit_cancel",
    "edit_ok"
};

std::map<int, std::map<std::string, int>> edit_action_enable_m =
{
    {
        Developer,
        {
            {"edit_add", 1},
            {"edit_add_before", 1},
            {"edit_add_after", 1},
            {"edit_copy", 1},
            {"edit_cut", 1},
            {"edit_paste", 1},
            {"edit_paste_before", 1},
            {"edit_paste_after", 1},
            {"edit_delete", 1},
            {"edit_save", 1},
            {"edit_export", 1},
            {"edit_cancel", 1},
            {"edit_ok", 1}
        }
    }
};

EditBar::EditBar(QWidget *parent) :
    QToolBar(parent)
{
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
    button_icons.push_back(ICON_EXPORT);
    button_icons.push_back(ICON_CANCEL);
    button_icons.push_back(ICON_OK);

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
    buttons.push_back(tb_export);
    buttons.push_back(tb_cancel);
    buttons.push_back(tb_ok);

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

    widget_actions.append({
        this->addWidget(tb_add),
        this->addWidget(tb_add_before),
        this->addWidget(tb_add_after),
        this->addSeparator(),
        this->addWidget(tb_copy),
        this->addWidget(tb_cut),
        this->addWidget(tb_paste),
        this->addWidget(tb_paste_before),
        this->addWidget(tb_paste_after),
        this->addSeparator(),
        this->addWidget(tb_delete),
        this->addSeparator(),
        this->addWidget(tb_save),
        this->addSeparator(),
        this->addWidget(tb_export),
        this->addWidget(tb_cancel),
        this->addWidget(tb_ok),
    });

    for (auto &[name, tb] : qzip(edit_tb_names, buttons)) {
        tb->setObjectName(name.c_str());
    }

    for (auto &[tooltip, tb] : qzip(tooltips_v, buttons)) {
        tb->setToolTip(tooltip.c_str());
    }

    for (auto &[name, action] : qzip(edit_action_names, widget_actions)) {
        action->setObjectName(name.c_str());
    }
}

EditBar::~EditBar() {

}

void EditBar::set_visible(std::map<QString, bool> edit_tb_enable_m) {
    for (auto &[name, visible] : edit_tb_enable_m) {
        int i = 0;
        for (auto &tb : buttons) {
            if (tb->objectName() == name) {
                tb->setVisible(visible);
                if (!visible) {
                    QString action_name = name;
                    action_name.remove(0, 3);
                    for (auto &action : widget_actions) {
                        if (action->objectName() == action_name) {
                            this->removeAction(action);
                            break;
                        }
                    }
                }
                break;
            }
            ++i;
        }
    }
}

void EditBar::disable_separators() {
    for (auto &action : widget_actions) {
        if (action->isSeparator()) {
            this->removeAction(action);
        }
    }
}
