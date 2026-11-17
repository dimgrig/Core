#ifndef STANDSERIALPROVIDER_H
#define STANDSERIALPROVIDER_H

#include <QObject>
#include <QString>

#include "core-widgets-lib-global.h"

class CORE_WIDGETS_LIB_EXPORT StandSerialProvider : public QObject
{
    Q_OBJECT
public:
    // Точка доступа к синглтону (потокобезопасный синглтон Мейерса)
    static StandSerialProvider& instance();

    // Запрещаем копирование и присваивание
    StandSerialProvider(const StandSerialProvider&) = delete;
    StandSerialProvider& operator=(const StandSerialProvider&) = delete;

    // Основной геттер серийного номера
    QString serial() const;

    // Принудительное обновление номера (например, если оператор ошибся при первом вводе)
    void resetSerial();

    // Метод доступен только при подключении модуля Qt Test
    static void setMockSerial(const QString& mockSerial);
private:
    explicit StandSerialProvider(QObject *parent = nullptr);
    ~StandSerialProvider() override = default;

    void initSerial();
    QString requestSerialFromUser();

    QString m_serial;
    static QString s_mockSerial;
    static constexpr const char* ENV_VAR_NAME = "STAND_SERIAL";
    static constexpr const char* SETTINGS_KEY = "Hardware/StandSerial";
};

#endif // STANDSERIALPROVIDER_H
