#ifndef DOUBLESPINBOXDIALOG_H
#define DOUBLESPINBOXDIALOG_H

#include <QComboBox>
#include <QDialog>
#include <QWidget>

#include "core-widgets-lib-global.h"

namespace Ui {
    class fdoublespinbox_dialog;
}

class CORE_WIDGETS_LIB_EXPORT DoubleSpinBoxDialog : public QDialog {
Q_OBJECT
public:
    explicit DoubleSpinBoxDialog(QWidget *parent = nullptr);
    ~DoubleSpinBoxDialog();

    double get_choice() const {return _choice;};
public slots:
    void set_caption(QString text);
    void set_value(double value);

    void on_finished();

private slots:

private:
    Ui::fdoublespinbox_dialog *ui;

    double _choice;
};

#endif // DOUBLESPINBOXDIALOG_H
