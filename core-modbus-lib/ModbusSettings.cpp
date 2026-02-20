#include "ModbusSettings.h"
#include "ui_modbus_settings.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QModbusDevice>
#include <QToolButton>
#include <QSettings>

#include "ModbusBase.h"

int ModbusSettingsConverter::modbus_state_from_str(QString state_str) {
    int state = QModbusDevice::UnconnectedState;

    state_str = state_str.toLower();
    if (state_str.contains("отключено")) {
        state = QModbusDevice::UnconnectedState;
    } else if (state_str.contains("подключение")) {
        state = QModbusDevice::ConnectingState;
    } else if (state_str.contains("подключено")) {
        state = QModbusDevice::ConnectedState;
    } else if (state_str.contains("отключение")) {
        state = QModbusDevice::ClosingState;
    }

    return state;
}

QString ModbusSettingsConverter::modbus_state_to_str(int state) {
    QString state_str;
    switch ((QModbusDevice::State)state) {
    case QModbusDevice::UnconnectedState:
        state_str = "Отключено";
        break;
    case QModbusDevice::ConnectingState:
        state_str = "Подключение";
        break;
    case QModbusDevice::ConnectedState:
        state_str = "Подключено";
        break;
    case QModbusDevice::ClosingState:
        state_str = "Отключение";
        break;
    default:
        state_str = "Отключено";
    }
    return state_str;
}

ModbusSettings ModbusSettingsConverter::modbus_settings_from_str(QString data) {
    ModbusSettings modbus_settings;
    QJsonDocument document = QJsonDocument::fromJson(data.toUtf8());
    if (!document.isNull()) {
        if (document.isObject()) {
            QJsonObject modbus_settings_object = document.object();
            modbus_settings.MODBUS_ENABLE = modbus_settings_object["MODBUS_ENABLE"].toBool();
            modbus_settings.MODBUS_POLL_TIMER = modbus_settings_object["MODBUS_POLL_TIMER"].toInt();
            modbus_settings.MODBUS_ADDRESS = modbus_settings_object["MODBUS_ADDRESS"].toInt();
            modbus_settings.MODBUS_TYPE = modbus_settings_object["MODBUS_TYPE"].toString();
            modbus_settings.MODBUS_IP = modbus_settings_object["MODBUS_IP"].toString();
            modbus_settings.MODBUS_PORT = modbus_settings_object["MODBUS_PORT"].toInt();
            modbus_settings.MODBUS_SERIAL_PORT = modbus_settings_object["MODBUS_SERIAL_PORT"].toString();
            modbus_settings.MODBUS_BAUDRATE = modbus_settings_object["MODBUS_BAUDRATE"].toInt();
            modbus_settings.MODBUS_DATA_BITS = modbus_settings_object["MODBUS_DATA_BITS"].toInt();
            modbus_settings.MODBUS_STOP_BITS = modbus_settings_object["MODBUS_STOP_BITS"].toInt();
            modbus_settings.MODBUS_PARITY = modbus_settings_object["MODBUS_PARITY"].toString();
            modbus_settings.MODBUS_STATE = modbus_settings_object["MODBUS_STATE"].toString();
            modbus_settings.MODBUS_FORMAT = modbus_settings_object["MODBUS_FORMAT"].toString();
        }
    }
    return modbus_settings;
}

QString ModbusSettingsConverter::modbus_settings_to_str(const ModbusSettings&  modbus_settings) {
    QJsonObject modbus_settings_object;

    modbus_settings_object.insert("MODBUS_ENABLE", QJsonValue(modbus_settings.MODBUS_ENABLE));
    modbus_settings_object.insert("MODBUS_POLL_TIMER", QJsonValue(modbus_settings.MODBUS_POLL_TIMER));
    modbus_settings_object.insert("MODBUS_ADDRESS", QJsonValue(modbus_settings.MODBUS_ADDRESS));
    modbus_settings_object.insert("MODBUS_TYPE", QJsonValue(modbus_settings.MODBUS_TYPE));
    modbus_settings_object.insert("MODBUS_IP", QJsonValue(modbus_settings.MODBUS_IP));
    modbus_settings_object.insert("MODBUS_PORT", QJsonValue(modbus_settings.MODBUS_PORT));
    modbus_settings_object.insert("MODBUS_SERIAL_PORT", QJsonValue(modbus_settings.MODBUS_SERIAL_PORT));
    modbus_settings_object.insert("MODBUS_BAUDRATE", QJsonValue(modbus_settings.MODBUS_BAUDRATE));
    modbus_settings_object.insert("MODBUS_DATA_BITS", QJsonValue(modbus_settings.MODBUS_DATA_BITS));
    modbus_settings_object.insert("MODBUS_STOP_BITS", QJsonValue(modbus_settings.MODBUS_STOP_BITS));
    modbus_settings_object.insert("MODBUS_PARITY", QJsonValue(modbus_settings.MODBUS_PARITY));
    modbus_settings_object.insert("MODBUS_STATE", QJsonValue(modbus_settings.MODBUS_STATE));
    modbus_settings_object.insert("MODBUS_FORMAT", QJsonValue(modbus_settings.MODBUS_FORMAT));

    QJsonDocument document(modbus_settings_object);
    return document.toJson(QJsonDocument::Compact);
}

QString ModbusSettingsConverter::getString(const QVariantMap& map, const QString& key, const QString& defaultValue)
{
    if (map.contains(key) && map.value(key).canConvert<QString>()) {
        return map.value(key).toString();
    }
    return defaultValue;
}

int ModbusSettingsConverter::getInt(const QVariantMap& map, const QString& key, int defaultValue)
{
    if (map.contains(key) && map.value(key).canConvert<int>()) {
        return map.value(key).toInt();
    }
    return defaultValue;
}

bool ModbusSettingsConverter::getBool(const QVariantMap& map, const QString& key, bool defaultValue)
{
    if (map.contains(key) && map.value(key).canConvert<bool>()) {
        return map.value(key).toBool();
    }
    return defaultValue;
}

ModbusSettings ModbusSettingsConverter::modbus_settings_from_variant_map(const QVariantMap &map) {
    ModbusSettings modbus_settings;

    // Используем значения по умолчанию, если ключи отсутствуют
    modbus_settings.MODBUS_ENABLE = ModbusSettingsConverter::getBool(map, "MODBUS_ENABLE", true);
    modbus_settings.MODBUS_POLL_TIMER = getInt(map, "MODBUS_POLL_TIMER", 500);
    modbus_settings.MODBUS_ADDRESS = getInt(map, "MODBUS_ADDRESS", 1);
    modbus_settings.MODBUS_TYPE = getString(map, "MODBUS_TYPE", "TCP");
    modbus_settings.MODBUS_IP = getString(map, "MODBUS_IP", "192.168.1.1");
    modbus_settings.MODBUS_PORT = getInt(map, "MODBUS_PORT", 32767);
    modbus_settings.MODBUS_SERIAL_PORT = getString(map, "MODBUS_SERIAL_PORT", "COM1");
    modbus_settings.MODBUS_BAUDRATE = getInt(map, "MODBUS_BAUDRATE", 115200);
    modbus_settings.MODBUS_DATA_BITS = getInt(map, "MODBUS_DATA_BITS", 8);
    modbus_settings.MODBUS_STOP_BITS = getInt(map, "MODBUS_STOP_BITS", 1);
    modbus_settings.MODBUS_PARITY = getString(map, "MODBUS_PARITY", "NoParity");
    modbus_settings.MODBUS_STATE = getString(map, "MODBUS_STATE", "");
    modbus_settings.MODBUS_FORMAT = getString(map, "MODBUS_FORMAT", "ABCD");

    return modbus_settings;
}

QVariantMap ModbusSettingsConverter::modbus_settings_to_variant_map(const ModbusSettings&  modbus_settings) {
    QVariantMap map;

    map["MODBUS_ENABLE"] = modbus_settings.MODBUS_ENABLE;
    map["MODBUS_POLL_TIMER"] = modbus_settings.MODBUS_POLL_TIMER;
    map["MODBUS_ADDRESS"] = modbus_settings.MODBUS_ADDRESS;
    map["MODBUS_TYPE"] = modbus_settings.MODBUS_TYPE;
    map["MODBUS_IP"] = modbus_settings.MODBUS_IP;
    map["MODBUS_PORT"] = modbus_settings.MODBUS_PORT;
    map["MODBUS_SERIAL_PORT"] = modbus_settings.MODBUS_SERIAL_PORT;
    map["MODBUS_BAUDRATE"] = modbus_settings.MODBUS_BAUDRATE;
    map["MODBUS_DATA_BITS"] = modbus_settings.MODBUS_DATA_BITS;
    map["MODBUS_STOP_BITS"] = modbus_settings.MODBUS_STOP_BITS;
    map["MODBUS_PARITY"] = modbus_settings.MODBUS_PARITY;
    map["MODBUS_STATE"] = modbus_settings.MODBUS_STATE;
    map["MODBUS_FORMAT"] = modbus_settings.MODBUS_FORMAT;

    return map;
}

ModbusSettingsUI::ModbusSettingsUI(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::fmodbus_settings)
{
    ui->setupUi(this);

    this->ui->tw->tabBar()->hide();

    this->initializeModbusComboboxes();

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

    connect(this->ui->tb_modbus_config, &QToolButton::clicked, this, &ModbusSettingsUI::load_modbus_config);

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
    QString state_str;
    switch (state) {
    case QModbusDevice::UnconnectedState:
        state_str = "Отключено";
        break;
    case QModbusDevice::ConnectingState:
        state_str = "Подключение";
        break;
    case QModbusDevice::ConnectedState:
        state_str = "Подключено";
        break;
    case QModbusDevice::ClosingState:
        state_str = "Отключение";
        break;
    }

    this->ui->le_state->setText(state_str);
    //qCDebug(logDebug()) << modbusDevice->state();
}


void ModbusSettingsUI::initializeModbusComboboxes() {
    this->ui->cb_connect_type->addItems(ModbusConnection_sl);
    this->ui->cb_baudrate->addItems(ModbusBaudRate_sl);
    this->ui->cb_data_bits->addItems(ModbusDataBits_sl);
    this->ui->cb_stop_bits->addItems(ModbusStopBits_sl);
    this->ui->cb_parity->addItems(ModbusParity_sl);
    this->ui->cb_format->addItems(ModbusFormat_sl);
}

void ModbusSettingsUI::type_changed(const QString &type) {
    auto connection_type = ModbusConnection_from_text(type.toStdString().c_str());
    int index = 0;
    if (connection_type == ModbusTCP) {
        index = 0;
        this->ui->tw->setCurrentIndex(index);
    } else {
        index = 1;
        this->ui->tw->setCurrentIndex(index);
    }

    QWidget* currentWidget = this->ui->tw->widget(index);

    // Получаем рекомендуемый размер содержимого
    int contentHeight = currentWidget->sizeHint().height();

    // Добавляем высоту панели вкладок
    int tabBarHeight = this->ui->tw->tabBar()->sizeHint().height();

    // Устанавливаем фиксированную высоту
    this->ui->tw->setFixedHeight(contentHeight + tabBarHeight + 2); // +2 для рамок
}

ModbusSettings ModbusSettingsUI::getSettingsFromUi() {
    ModbusSettings settings;

    settings.MODBUS_POLL_TIMER = ui->sb_poll_timer->value();
    settings.MODBUS_ADDRESS = ui->le_address->text().toInt();

    settings.MODBUS_TYPE = ModbusConnection_to_text(ui->cb_connect_type->currentIndex());

    settings.MODBUS_IP = ui->le_ip->text();
    settings.MODBUS_PORT = ui->le_port->text().toInt();

    settings.MODBUS_SERIAL_PORT = ui->le_com_port->text();
    settings.MODBUS_BAUDRATE =  ModbusBaudRate_from_text(this->ui->cb_baudrate->currentText().toStdString().c_str());
    settings.MODBUS_DATA_BITS = ModbusDataBits_from_text(this->ui->cb_data_bits->currentText().toStdString().c_str());
    settings.MODBUS_STOP_BITS = ModbusStopBits_from_text(this->ui->cb_stop_bits->currentText().toStdString().c_str());
    settings.MODBUS_PARITY = ModbusParity_to_text(this->ui->cb_parity->currentIndex());
    settings.MODBUS_FORMAT = this->ui->cb_format->currentText();

    return settings;
}

void ModbusSettingsUI::setSettingsToUi(const ModbusSettings& settings) {
    ui->sb_poll_timer->setValue(settings.MODBUS_POLL_TIMER);
    ui->le_address->setText(QString::number(settings.MODBUS_ADDRESS));

    ui->cb_connect_type->setCurrentIndex(static_cast<int>(ModbusConnection_from_text(settings.MODBUS_TYPE.toStdString().c_str())));

    ui->le_ip->setText(settings.MODBUS_IP);
    ui->le_port->setText(QString::number(settings.MODBUS_PORT));

    ui->le_com_port->setText(settings.MODBUS_SERIAL_PORT);
    ui->cb_baudrate->setCurrentText(ModbusBaudRate_to_text(static_cast<ModbusBaudRate>(settings.MODBUS_BAUDRATE)));
    ui->cb_data_bits->setCurrentText(ModbusDataBits_to_text(static_cast<ModbusDataBits>(settings.MODBUS_DATA_BITS)));
    ui->cb_stop_bits->setCurrentText(ModbusStopBits_to_text(static_cast<ModbusStopBits>(settings.MODBUS_STOP_BITS)));
    ui->cb_parity->setCurrentText(ModbusParity_to_text(ModbusParity_from_text(settings.MODBUS_PARITY.toStdString().c_str())));
    ui->cb_format->setCurrentText(settings.MODBUS_FORMAT);
}

void ModbusSettingsUI::read_config(ModbusSettings modbus_settings){
    this->setSettingsToUi(modbus_settings);
}

ModbusSettings ModbusSettingsUI::write_config(){
    ModbusSettings modbus_settings = this->getSettingsFromUi();

    QSettings config("config.ini", QSettings::IniFormat);
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

void ModbusSettingsUI::load_modbus_config() {
    auto path = this->ui->le_config->text();
    QDir dir;
    QFileInfo fileinfo(path);
    if (fileinfo.isFile() && (path != ":json/json/modbus.json")) {
        dir = fileinfo.absoluteDir();
    } else {
        dir.setPath(QApplication::applicationDirPath());
    }

    QString filename = QFileDialog::getOpenFileName(this, "Файл конфигурации регистров", dir.path(), "*.json");
    fileinfo = QFileInfo(filename);
    if (fileinfo.isFile()) {
        if (fileinfo.suffix() == "json") {
            this->ui->le_config->setText(filename);
            emit config_changed(filename);
        }
    }
}

void ModbusSettingsUI::set_config(QString config) {
    this->ui->le_config->setText(config);
}
