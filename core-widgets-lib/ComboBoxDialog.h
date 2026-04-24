#ifndef COMBOBOXDIALOG_H
#define COMBOBOXDIALOG_H

#include <QComboBox>
#include <QDialog>
#include <QWidget>

#include "core-widgets-lib-global.h"

namespace Ui {
    class fcombobox_dialog;
}

class CORE_WIDGETS_LIB_EXPORT ComboBoxDialog : public QDialog {
Q_OBJECT
public:
    explicit ComboBoxDialog(QWidget *parent = nullptr);
    ~ComboBoxDialog();

    int get_choice() const {return _choice;};
public slots:
    void set_caption(QString text);
    void set_items(QStringList &items);
    void set_item(QString item);
    void set_index(int index);

    void on_finished();

private slots:

private:
    Ui::fcombobox_dialog *ui;

    int _choice;
};

#endif // COMBOBOXDIALOG_H
