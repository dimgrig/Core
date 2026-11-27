#include "NetworkUtils.h"
#include <QNetworkInterface>
#include <QNetworkAddressEntry>

#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
#include <QNetworkInformation>
#elif QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
// Для Qt 6.0 - 6.3 QNetworkInformation нет, используем только интерфейсы
#else
#include <QNetworkConfigurationManager>
#endif

bool NetworkUtils::hasActiveNetworkInterface() {
    const auto interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface &iface : interfaces) {
        // Интерфейс должен быть включен и не быть loopback (127.0.0.1)
        if (iface.flags().testFlag(QNetworkInterface::IsUp) &&
            !iface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            
            // Проверяем, есть ли у него назначенный IP-адрес (IPv4 или IPv6)
            for (const QNetworkAddressEntry &entry : iface.addressEntries()) {
                if (entry.ip().protocol() != QAbstractSocket::UnknownNetworkLayerProtocol) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool NetworkUtils::isOnline() {
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
    // Пытаемся загрузить бэкенд, поддерживающий определение Reachability
    if (!QNetworkInformation::instance()) {
        QNetworkInformation::loadBackendByFeatures(QNetworkInformation::Feature::Reachability);
    }

    if (auto *netInfo = QNetworkInformation::instance()) {
        return (netInfo->reachability() == QNetworkInformation::Reachability::Online);
    } else {
        // Fallback: плагин не загрузился (не задеплоен или ОС не поддерживается).
        return hasActiveNetworkInterface();
    }
#elif QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return hasActiveNetworkInterface();
#else
    // Qt 5
    QNetworkConfigurationManager mgr;
    return mgr.isOnline();
#endif
}