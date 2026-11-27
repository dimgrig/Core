#ifndef NETWORKUTILS_H
#define NETWORKUTILS_H

#include <QObject>
#include "core-utils-lib-global.h"

class CORE_UTILS_LIB_EXPORT NetworkUtils : public QObject {
    Q_OBJECT
public:
    explicit NetworkUtils(QObject *parent = nullptr) = delete; // Запрещаем создание экземпляров

    /**
     * @brief Проверяет наличие активного подключения к интернету.
     * Поддерживает Qt 5, Qt 6.0+ и Qt 6.4+ (с QNetworkInformation).
     */
    static bool isOnline();

private:
    static bool hasActiveNetworkInterface();
};

#endif // NETWORKUTILS_H