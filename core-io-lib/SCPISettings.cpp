#include "SCPISettings.h"
#include "ui_scpi_settings.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QToolButton>
#include <QSettings>

#include "CoreConfig.h"

SCPISettingsUI::SCPISettingsUI(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::fscpi_settings)
{
    ui->setupUi(this);

    this->initializeComboboxes();

    connect(this->ui->cb_connect_type, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SCPISettingsUI::settings_changed);
    connect(this->ui->le_ip, &QLineEdit::editingFinished, this, &SCPISettingsUI::settings_changed);
    connect(this->ui->le_port, &QLineEdit::editingFinished, this, &SCPISettingsUI::settings_changed);
    connect(this->ui->le_com_port, &QLineEdit::editingFinished, this, &SCPISettingsUI::settings_changed);
    connect(this->ui->cb_baudrate, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SCPISettingsUI::settings_changed);
    connect(this->ui->cb_data_bits, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SCPISettingsUI::settings_changed);
    connect(this->ui->cb_stop_bits, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SCPISettingsUI::settings_changed);
    connect(this->ui->cb_parity, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SCPISettingsUI::settings_changed);

    //this->set_reconfig();

    connect(this->ui->cb_connect_type, &QComboBox::currentTextChanged, this, &SCPISettingsUI::type_changed);
    this->type_changed(this->ui->cb_connect_type->currentText());
}

SCPISettingsUI::~SCPISettingsUI() {
    delete ui;
}

void SCPISettingsUI::initializeComboboxes() {
    this->ui->cb_connect_type->addItems(ConnectionType_sl);
    this->ui->cb_baudrate->addItems(SerialBaudRate_sl);
    this->ui->cb_data_bits->addItems(SerialDataBits_sl);
    this->ui->cb_stop_bits->addItems(SerialStopBits_sl);
    this->ui->cb_parity->addItems(SerialParity_sl);
}

void SCPISettingsUI::type_changed(const QString &type) {
    auto connection_type = ConnectionType_from_text(type.toStdString().c_str());
    if (connection_type == ConnectionTypeTCP) {
        this->ui->l_ip->show();
        this->ui->le_ip->show();
        this->ui->l_port->show();
        this->ui->le_port->show();

        this->ui->l_com_port->hide();
        this->ui->le_com_port->hide();
        this->ui->l_baudrate->hide();
        this->ui->cb_baudrate->hide();
        this->ui->l_data_bits->hide();
        this->ui->cb_data_bits->hide();
        this->ui->l_stop_bits->hide();
        this->ui->cb_stop_bits->hide();
        this->ui->l_parity->hide();
        this->ui->cb_parity->hide();
    } else {
        this->ui->l_ip->hide();
        this->ui->le_ip->hide();
        this->ui->l_port->hide();
        this->ui->le_port->hide();

        this->ui->l_com_port->show();
        this->ui->le_com_port->show();
        this->ui->l_baudrate->show();
        this->ui->cb_baudrate->show();
        this->ui->l_data_bits->show();
        this->ui->cb_data_bits->show();
        this->ui->l_stop_bits->show();
        this->ui->cb_stop_bits->show();
        this->ui->l_parity->show();
        this->ui->cb_parity->show();
    }
}

SCPISettings SCPISettingsUI::getSettingsFromUi() {
    SCPISettings settings;

    settings.SCPI_TYPE = ConnectionType_to_text(ui->cb_connect_type->currentIndex());

    settings.SCPI_IP = ui->le_ip->text();
    settings.SCPI_PORT = ui->le_port->text().toInt();

    settings.SCPI_SERIAL_PORT = ui->le_com_port->text();
    settings.SCPI_BAUDRATE =  SerialBaudRate_from_text(this->ui->cb_baudrate->currentText().toStdString().c_str());
    settings.SCPI_DATA_BITS = SerialDataBits_from_text(this->ui->cb_data_bits->currentText().toStdString().c_str());
    settings.SCPI_STOP_BITS = SerialStopBits_from_text(this->ui->cb_stop_bits->currentText().toStdString().c_str());
    settings.SCPI_PARITY = SerialParity_to_text(this->ui->cb_parity->currentIndex());

    return settings;
}

void SCPISettingsUI::setSettingsToUi(const SCPISettings& settings) {
    ui->cb_connect_type->setCurrentIndex(static_cast<int>(ConnectionType_from_text(settings.SCPI_TYPE.toStdString().c_str())));

    ui->le_ip->setText(settings.SCPI_IP);
    ui->le_port->setText(QString::number(settings.SCPI_PORT));

    ui->le_com_port->setText(settings.SCPI_SERIAL_PORT);
    ui->cb_baudrate->setCurrentText(SerialBaudRate_to_text(static_cast<SerialBaudRate>(settings.SCPI_BAUDRATE)));
    ui->cb_data_bits->setCurrentText(SerialDataBits_to_text(static_cast<SerialDataBits>(settings.SCPI_DATA_BITS)));
    ui->cb_stop_bits->setCurrentText(SerialStopBits_to_text(static_cast<SerialStopBits>(settings.SCPI_STOP_BITS)));
    ui->cb_parity->setCurrentText(SerialParity_to_text(SerialParity_from_text(settings.SCPI_PARITY.toStdString().c_str())));
}

void SCPISettingsUI::read_config(SCPISettings settings){
    this->setSettingsToUi(settings);
}

SCPISettings SCPISettingsUI::write_config(){
    SCPISettings settings = this->getSettingsFromUi();

    QSettings config(CoreConfig::configFile(), QSettings::IniFormat);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    config.setIniCodec("UTF-8"); //Windows-1251
#endif

    config.beginGroup("SCPI");
    config.setValue("SCPI_ENABLE", settings.SCPI_ENABLE);
    config.setValue("SCPI_TYPE", settings.SCPI_TYPE);
    config.setValue("SCPI_IP", settings.SCPI_IP);
    config.setValue("SCPI_PORT", settings.SCPI_PORT);
    config.setValue("SCPI_SERIAL_PORT", settings.SCPI_SERIAL_PORT);
    config.setValue("SCPI_BAUDRATE", settings.SCPI_BAUDRATE);
    config.setValue("SCPI_DATA_BITS", settings.SCPI_DATA_BITS);
    config.setValue("SCPI_STOP_BITS", settings.SCPI_STOP_BITS);
    config.setValue("SCPI_PARITY", settings.SCPI_PARITY);
    config.endGroup();

    return settings;
}
