#include "IconManager.h"

#include <QPainter>
#include <QtSvg/QtSvg>
#include <QtSvg/QSvgRenderer>

// QMap<QString, QIcon> IconManager::iconCache() {
//     return s_iconCache;
// }

// QMap<QString, QPixmap> IconManager::pixmapCache() {
//     return s_pixmapCache;
// }

// QMutex& IconManager::iconMutex() {
//     return s_iconMutex;
// }

// QMutex& IconManager::pixmapMutex() {
//     return s_pixmapMutex;
// }

QString IconManager::generatePixmapCacheKey(const QString& path, const QSize& size) {
    if (size.isEmpty()) {
        return path;
    }
    return QString("%1_%2x%3").arg(path).arg(size.width()).arg(size.height());
}

// QIcon IconManager::createScalableIconFromSvg(const QString& path)
// {
//     QIcon icon;
//     QSvgRenderer renderer(path);

//     if (!renderer.isValid()) {
//         return icon;
//     }

//     // Добавляем несколько предварительно отрендеренных размеров
//     QList<QSize> sizes = {ICON_SIZE_16, ICON_SIZE_32, ICON_SIZE_64};

//     for (const QSize& size : sizes) {
//         QImage image(size, QImage::Format_ARGB32);
//         image.fill(Qt::transparent);

//         QPainter painter(&image);
//         renderer.render(&painter);

//         icon.addPixmap(QPixmap::fromImage(image));
//     }

//     return icon;
// }

QIcon IconManager::getIcon(const QString& path) {
    QMutexLocker locker(&s_iconMutex);

    // Проверяем кэш
    if (s_iconCache.contains(path)) {
        return s_iconCache.value(path);
    }

    // if (s_pixmapCache.contains(path)) {
    //     return QIcon(s_pixmapCache.value(path));
    // }

    // Загружаем и кэшируем
    QIcon icon;
    if (path.endsWith(".svg", Qt::CaseInsensitive)) {
        // Для SVG используем QIcon с поддержкой масштабирования
        //icon = createScalableIconFromSvg(path);
        QList<QSize> sizes = {ICON_SIZE_16, ICON_SIZE_24, ICON_SIZE_32, ICON_SIZE_64};

        for (const QSize& size : sizes) {
            icon.addFile(path, size);
        }
    } else {
        // Для растовых изображений
        icon = QIcon(path);
    }

    if (icon.isNull()) {
        // Возвращаем пустую иконку вместо падения
        return QIcon();
    }

    s_iconCache.insert(path, icon);
    return icon;
}

QPixmap IconManager::getPixmap(const QString& path, const QSize& size) {
    QString cacheKey = generatePixmapCacheKey(path, size);

    QMutexLocker locker(&s_pixmapMutex);

    // Проверяем кэш
    if (s_pixmapCache.contains(cacheKey)) {
        return s_pixmapCache.value(cacheKey);
    }

    // Загружаем пиксмап
    QPixmap pixmap;

    if (path.startsWith(":/")) {
        // Загрузка из ресурсов
        pixmap.load(path);
    } else {
        // Загрузка из файловой системы
        pixmap = QPixmap(path);
    }

    if (pixmap.isNull()) {
        // Создаем placeholder (небольшой красный квадрат для отладки)
        pixmap = QPixmap(size.isValid() ? size : QSize(512, 512));
        pixmap.fill(Qt::red);
        return pixmap;
    }

    // Масштабируем если нужно
    if (size.isValid() && !size.isEmpty() && pixmap.size() != size) {
        // Используем плавное масштабирование для сохранения качества
        pixmap = pixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    // Кэшируем
    s_pixmapCache.insert(cacheKey, pixmap);

    return pixmap;
}

QPixmap IconManager::getPixmap(const QString& path, int width, int height) {
    return getPixmap(path, QSize(width, height));
}

void IconManager::preloadIcons(const QStringList& iconPaths) {
    for (const QString& path : iconPaths) {
        // Предзагружаем иконку
        getIcon(path);

        // Также предзагружаем пиксмап в стандартном размере
        if (!m_defaultIconSize.isEmpty()) {
            getPixmap(path, m_defaultIconSize);
        }
    }
}

void IconManager::preloadAllStandardIcons() {
    // Список всех стандартных иконок приложения
    QStringList standardIcons = {
        // Базовые иконки
        ":/icons/add.svg",
        ":/icons/add_before.svg",
        ":/icons/add_after.svg",
        ":/icons/copy.svg",
        ":/icons/cut.svg",
        ":/icons/paste.svg",
        ":/icons/insert_before.svg",
        ":/icons/insert_after.svg",
        ":/icons/delete.svg",
        ":/icons/save.svg",
        ":/icons/save_as.svg",
        ":/icons/export.svg",
        ":/icons/cancel.svg",
        ":/icons/ok.svg",
        ":/icons/close.svg",
        ":/icons/open.svg",
        ":/icons/apply.svg",

        // Иконки навигации
        ":/icons/play.svg",
        ":/icons/pause.svg",
        ":/icons/back.svg",
        ":/icons/forward.svg",
        ":/icons/stop.svg",
        ":/icons/run.svg",
        ":/icons/run_all.svg",

        // Иконки устройств
        ":/icons/sensor.svg",
        ":/icons/query_settings.svg",
        ":/icons/chart.svg",
        ":/icons/locked.svg",
        ":/icons/unlocked.svg",

        // Иконки отчетов
        ":/icons/print.svg",
        ":/icons/report.svg",
        ":/icons/statistic.svg",
        ":/icons/oscillogram.svg",
        ":/icons/histogram.svg",
        ":/icons/calibration.svg",

        // Иконки состояния
        ":/icons/led_red_on.svg",
        ":/icons/led_red_off.svg",
        ":/icons/led_green_on.svg",
        ":/icons/led_green_off.svg"
    };

    preloadIcons(standardIcons);
}

void IconManager::clearPixmapCache() {
    QMutexLocker locker(&s_pixmapMutex);
    s_pixmapCache.clear();
    // Также очищаем системный кэш Qt
    QPixmapCache::clear();
}

void IconManager::clearIconCache() {
    QMutexLocker locker(&s_iconMutex);
    s_iconCache.clear();
}

void IconManager::clearAllCaches() {
    clearIconCache();
    clearPixmapCache();
}

QString IconManager::cacheStatistics() {
    QMutexLocker iconLocker(&s_iconMutex);
    QMutexLocker pixmapLocker(&s_pixmapMutex);

    // Оценка использования памяти (очень приблизительная)
    qint64 iconMemory = 0;
    qint64 pixmapMemory = 0;

    for (const QPixmap& pixmap : s_pixmapCache) {
        pixmapMemory += pixmap.width() * pixmap.height() * 4; // ARGB = 4 байта
    }

    QString stats;
    stats += QString("Icon Manager Cache Statistics:\n");
    stats += QString("  Icons cached: %1\n").arg(s_iconCache.size());
    stats += QString("  Pixmaps cached: %1\n").arg(s_pixmapCache.size());
    stats += QString("  Estimated pixmap memory: %1 MB\n").arg(pixmapMemory / 1024 / 1024);
    stats += QString("  Qt pixmap cache limit: %1 KB\n").arg(QPixmapCache::cacheLimit());
    stats += QString("  Default icon size: %1x%2").arg(m_defaultIconSize.width()).arg(m_defaultIconSize.height());

    return stats;
}

void IconManager::setDefaultIconSize(const QSize& size) {
    m_defaultIconSize = size;
}

QSize IconManager::defaultIconSize() {
    return m_defaultIconSize;
}

// void IconManager::setSvgRendererCacheSize(int size) {
//     // Если используете QtSvg, можно настроить кэш рендерера
// #ifdef QT_SVG_LIB
//     QSvgRenderer::setCacheSize(size * 1024); // размер в KB
//     qDebug() << "SVG renderer cache set to" << size << "KB";
// #endif
// }
