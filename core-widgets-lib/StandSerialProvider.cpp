#include "StandSerialProvider.h"

#include <QSettings>
#include <QInputDialog>
#include <QMessageBox>
#include <QApplication>

QString StandSerialProvider::s_mockSerial;

StandSerialProvider& StandSerialProvider::instance()
{
    static StandSerialProvider instance;
    return instance;
}

StandSerialProvider::StandSerialProvider(QObject *parent)
    : QObject(parent)
{
    initSerial();
}

QString StandSerialProvider::serial() const
{
    return m_serial;
}

void StandSerialProvider::initSerial()
{
    // 0. Мок для тестов
    if (!s_mockSerial.isEmpty()) {
        m_serial = s_mockSerial;
        return;
    }

    // 1. Ищем в переменных окружения ОС
    QString envSerial = qEnvironmentVariable(ENV_VAR_NAME).trimmed();
    if (!envSerial.isEmpty()) {
        m_serial = envSerial;
        return;
    }

    // 2. Ищем в сохраненных настройках ОС (реестр в Windows / .conf в Linux)
    QSettings settings;
    QString savedSerial = settings.value(SETTINGS_KEY).toString().trimmed();
    if (!savedSerial.isEmpty()) {
        m_serial = savedSerial;
        // Пробрасываем в окружение текущего процесса на случай, если его читают дочерние утилиты
        qputenv(ENV_VAR_NAME, m_serial.toUtf8());
        return;
    }

    // 3. Если нигде нет — запрашиваем у пользователя
    m_serial = requestSerialFromUser();
    if (!m_serial.isEmpty()) {
        settings.setValue(SETTINGS_KEY, m_serial);
        qputenv(ENV_VAR_NAME, m_serial.toUtf8());
    }
}

QString StandSerialProvider::requestSerialFromUser()
{
    while (true) {
        bool ok = false;
        QString input = QInputDialog::getText(
                            QApplication::activeWindow(),
                            tr("Первичная настройка оборудования"),
                            tr("Не задан заводской номер поверочного стенда.\nВведите серийный номер устройства:"),
                            QLineEdit::Normal,
                            QString(),
                            &ok
                            ).trimmed();

        if (ok && !input.isEmpty()) {
            return input;
        }

        // Если нажали Cancel или оставили поле пустым — предупреждаем, что номер обязателен
        QMessageBox::warning(
            QApplication::activeWindow(),
            tr("Внимание"),
            tr("Заводской номер стенда обязателен для формирования протоколов поверки.")
            );
    }
}

void StandSerialProvider::resetSerial()
{
    QSettings settings;
    settings.remove(SETTINGS_KEY);
    qunsetenv(ENV_VAR_NAME);
    m_serial = requestSerialFromUser();
    if (!m_serial.isEmpty()) {
        settings.setValue(SETTINGS_KEY, m_serial);
        qputenv(ENV_VAR_NAME, m_serial.toUtf8());
    }
}

void StandSerialProvider::setMockSerial(const QString& mockSerial)
{
    s_mockSerial = mockSerial;
}
