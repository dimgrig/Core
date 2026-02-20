#include "ComboBoxDialog.h"
#include "ui_combobox_dialog.h"

ComboBoxDialog::ComboBoxDialog(QWidget *parent) :
        QDialog(parent),
        ui(new Ui::fcombobox_dialog) {
    ui->setupUi(this);

    this->setWindowFlags(Qt::Window);

    this->setWindowTitle("");

    connect(this, &QDialog::finished, this, &ComboBoxDialog::on_finished);
}

ComboBoxDialog::~ComboBoxDialog()
{
    delete ui;
}

void ComboBoxDialog::set_caption(QString text) {
    this->ui->l_combobox_caption->setText(text);
}

void ComboBoxDialog::set_items(QStringList &items) {
    this->ui->cb_choice->clear();
    this->ui->cb_choice->addItems(items);

    this->_choice = this->ui->cb_choice->currentIndex();
}

void ComboBoxDialog::set_item(QString item) {
    this->ui->cb_choice->setCurrentText(item);
}

void ComboBoxDialog::set_index(int index) {
    this->ui->cb_choice->setCurrentIndex(index);
}

void ComboBoxDialog::on_finished() {
    this->_choice = this->ui->cb_choice->currentIndex();
}
