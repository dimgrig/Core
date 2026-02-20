#include "DoubleSpinBoxDialog.h"
#include "ui_doublespinbox_dialog.h"

DoubleSpinBoxDialog::DoubleSpinBoxDialog(QWidget *parent) :
        QDialog(parent),
        ui(new Ui::fdoublespinbox_dialog) {
    ui->setupUi(this);

    this->setWindowFlags(Qt::Window);

    this->setWindowTitle("");

    connect(this, &QDialog::finished, this, &DoubleSpinBoxDialog::on_finished);
}

DoubleSpinBoxDialog::~DoubleSpinBoxDialog()
{
    delete ui;
}

void DoubleSpinBoxDialog::set_caption(QString text) {
    this->ui->l_doublespinbox_caption->setText(text);
}

void DoubleSpinBoxDialog::set_value(double value) {
    this->ui->dsb_choice->setValue(value);
    this->_choice = this->ui->dsb_choice->value();
}

void DoubleSpinBoxDialog::on_finished() {
    this->_choice = this->ui->dsb_choice->value();
}
