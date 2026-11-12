#include "ModbusSettingsUI.h"
#include "ui_modbus_settings.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QToolButton>
#include <QSettings>

#include "CoreConfig.h"
#include <core-modbus-lib/ModbusBase.h>

ModbusSettingsUI::ModbusSettingsUI(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::fmodbus_settings)
{
    ui->setupUi(this);

    this->initializeComboboxes();

    connect(this->ui->cb_connect_type, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModbusSettingsUI::settings_changed);
    connect(this->ui->le_address, &QLineEdit::editingFinished, this, &ModbusSettingsUI::settings_changed);
    connect(this->ui->le_ip, &QLineEdit::editingFinished, this, &ModbusSettingsUI::settings_changed);
    connect(this->ui->le_port, &QLineEdit::editingFinished, this, &ModbusSettingsUI::settings_changed);
    connect(this->ui->le_com_port, &QLineEdit::editingFinished, this, &ModbusSettingsUI::settings_changed);
    connect(this->ui->cb_baudrate, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModbusSettingsUI::settings_changed);
    connect(this->ui->cb_data_bits, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModbusSettingsUI::settings_changed);
    connect(this->ui->cb_stop_bits, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModbusSettingsUI::settings_changed);
    connect(this->ui->cb_parity, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModbusSettingsUI::settings_changed);
    connect(this->ui->cb_format, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModbusSettingsUI::settings_changed);
    connect(this->ui->sb_poll_timer, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value){
        emit poll_timer_changed(value);
    });

    connect(this->ui->tb_modbus_config, &QToolButton::clicked, this, &ModbusSettingsUI::load_config);

    this->set_reconfig();

    connect(this->ui->cb_connect_type, &QComboBox::currentTextChanged, this, &ModbusSettingsUI::type_changed);
    this->type_changed(this->ui->cb_connect_type->currentText());
}

ModbusSettingsUI::~ModbusSettingsUI() {
    delete ui;
}

void ModbusSettingsUI::set_reconfig(bool reconfig) {
    this->_reconfig = reconfig;
    if (this->_reconfig) {
        this->ui->le_config->show();
        this->ui->tb_modbus_config->show();
    } else {
        this->ui->le_config->hide();
        this->ui->tb_modbus_config->hide();
    }
}

int ModbusSettingsUI::get_poll_timer() const {
    return this->ui->sb_poll_timer->value();
}

void ModbusSettingsUI::onModbusStateChanged(int state) {
    this->ui->le_state->setText(ModbusSettingsConverter::modbus_state_to_str(state));
}

void ModbusSettingsUI::initializeComboboxes() {
    this->ui->cb_connect_type->addItems(ConnectionType_sl);
    this->ui->cb_baudrate->addItems(SerialBaudRate_sl);
    this->ui->cb_data_bits->addItems(SerialDataBits_sl);
    this->ui->cb_stop_bits->addItems(SerialStopBits_sl);
    this->ui->cb_parity->addItems(SerialParity_sl);
    this->ui->cb_format->addItems(BytesFormat_sl);
}

void ModbusSettingsUI::type_changed(const QString &type) {
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

ModbusSettings ModbusSettingsUI::getSettingsFromUi() {
    ModbusSettings settings;

    settings.MODBUS_POLL_TIMER = ui->sb_poll_timer->value();
    settings.MODBUS_ADDRESS = ui->le_address->text().toInt();

    settings.MODBUS_TYPE = ConnectionType_to_text(ui->cb_connect_type->currentIndex());

    settings.MODBUS_IP = ui->le_ip->text();
    settings.MODBUS_PORT = ui->le_port->text().toInt();

    settings.MODBUS_SERIAL_PORT = ui->le_com_port->text();
    settings.MODBUS_BAUDRATE =  SerialBaudRate_from_text(this->ui->cb_baudrate->currentText().toStdString().c_str());
    settings.MODBUS_DATA_BITS = SerialDataBits_from_text(this->ui->cb_data_bits->currentText().toStdString().c_str());
    settings.MODBUS_STOP_BITS = SerialStopBits_from_text(this->ui->cb_stop_bits->currentText().toStdString().c_str());
    settings.MODBUS_PARITY = SerialParity_to_text(this->ui->cb_parity->currentIndex());
    settings.MODBUS_FORMAT = this->ui->cb_format->currentText();

    return settings;
}

void ModbusSettingsUI::setSettingsToUi(const ModbusSettings& settings) {
    ui->sb_poll_timer->setValue(settings.MODBUS_POLL_TIMER);
    ui->le_address->setText(QString::number(settings.MODBUS_ADDRESS));

    ui->cb_connect_type->setCurrentIndex(static_cast<int>(ConnectionType_from_text(settings.MODBUS_TYPE.toStdString().c_str())));

    ui->le_ip->setText(settings.MODBUS_IP);
    ui->le_port->setText(QString::number(settings.MODBUS_PORT));

    ui->le_com_port->setText(settings.MODBUS_SERIAL_PORT);
    ui->cb_baudrate->setCurrentText(SerialBaudRate_to_text(static_cast<SerialBaudRate>(settings.MODBUS_BAUDRATE)));
    ui->cb_data_bits->setCurrentText(SerialDataBits_to_text(static_cast<SerialDataBits>(settings.MODBUS_DATA_BITS)));
    ui->cb_stop_bits->setCurrentText(SerialStopBits_to_text(static_cast<SerialStopBits>(settings.MODBUS_STOP_BITS)));
    ui->cb_parity->setCurrentText(SerialParity_to_text(SerialParity_from_text(settings.MODBUS_PARITY.toStdString().c_str())));
    ui->cb_format->setCurrentText(settings.MODBUS_FORMAT);
}

void ModbusSettingsUI::read_config(ModbusSettings settings){
    this->setSettingsToUi(settings);
}

ModbusSettings ModbusSettingsUI::write_config(){
    ModbusSettings modbus_settings = this->getSettingsFromUi();

    QSettings config(CoreConfig::configFile(), QSettings::IniFormat);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    config.setIniCodec("UTF-8"); //Windows-1251
#endif

    config.beginGroup("modbus");
    config.setValue("MODBUS_ENABLE", modbus_settings.MODBUS_ENABLE);
    config.setValue("MODBUS_POLL_TIMER", modbus_settings.MODBUS_POLL_TIMER);
    config.setValue("MODBUS_ADDRESS", modbus_settings.MODBUS_ADDRESS);
    config.setValue("MODBUS_TYPE", modbus_settings.MODBUS_TYPE);
    config.setValue("MODBUS_IP", modbus_settings.MODBUS_IP);
    config.setValue("MODBUS_PORT", modbus_settings.MODBUS_PORT);
    config.setValue("MODBUS_SERIAL_PORT", modbus_settings.MODBUS_SERIAL_PORT);
    config.setValue("MODBUS_BAUDRATE", modbus_settings.MODBUS_BAUDRATE);
    config.setValue("MODBUS_DATA_BITS", modbus_settings.MODBUS_DATA_BITS);
    config.setValue("MODBUS_STOP_BITS", modbus_settings.MODBUS_STOP_BITS);
    config.setValue("MODBUS_PARITY", modbus_settings.MODBUS_PARITY);
    config.setValue("MODBUS_FORMAT", modbus_settings.MODBUS_FORMAT);
    config.endGroup();

    return modbus_settings;
}

void ModbusSettingsUI::load_config() {
    auto path = this->ui->le_config->text();
    QDir dir;
    QFileInfo fileinfo(path);
    if (fileinfo.isFile() && (path != ":json/json/modbus.json")) {
        dir = fileinfo.absoluteDir();
    } else {
        dir.setPath(QApplication::applicationDirPath());
    }

    QString filename = QFileDialog::getOpenFileName(
        this,
        "Файл конфигурации регистров",
        dir.path(),
        "Конфигурация (*.json *.csv);;JSON (*.json);;CSV (*.csv)");
    fileinfo = QFileInfo(filename);
    if (fileinfo.isFile()) {
        const QString suffix = fileinfo.suffix().toLower();
        if (suffix == "json" || suffix == "csv") {
            this->ui->le_config->setText(filename);
            emit config_changed(filename);
        }
    }
}

void ModbusSettingsUI::set_config(QString config) {
    this->ui->le_config->setText(config);
}

void ModbusSettingsUI::setSettingsEnabled(bool enabled) {
    // Блокируем всё, кроме поля состояния подключения.
    // findChildren<QWidget*>() не возвращает сам виджет, поэтому
    // setEnabled(false) не блокирует сам ModbusSettingsUI.
    for (QWidget* w : findChildren<QWidget*>()) {
        const QString name = w->objectName();
        if (name == "le_state" || name == "l_state") {
            continue;
        }
        w->setEnabled(enabled);
    }
}
