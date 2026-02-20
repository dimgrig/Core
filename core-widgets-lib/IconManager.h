#ifndef ICON_MANAGER_H
#define ICON_MANAGER_H

#include <QIcon>
#include <QMap>
#include <QMutex>
#include <QPixmap>
#include <QPixmapCache>
#include <QSize>

#include "icons.h"

#include "core-widgets-lib-global.h"

class CORE_WIDGETS_LIB_EXPORT IconManager {
public:
    // Получение иконки (кэшируется автоматически QIcon)
    static QIcon getIcon(const QString& path);

    // Получение пиксмапа с возможным масштабированием
    static QPixmap getPixmap(const QString& path, const QSize& size = QSize());

    // Получение пиксмапа по размеру (ширина, высота)
    static QPixmap getPixmap(const QString& path, int width, int height);

    // Предзагрузка набора иконок
    static void preloadIcons(const QStringList& iconPaths);

    // Предзагрузка всех стандартных иконок приложения
    static void preloadAllStandardIcons();

    // Очистка кэша пиксмапов
    static void clearPixmapCache();

    // Очистка кэша иконок
    static void clearIconCache();

    // Очистка всех кэшей
    static void clearAllCaches();

    // Получение статистики использования кэша
    static QString cacheStatistics();

    // Установка глобального размера иконок по умолчанию
    static void setDefaultIconSize(const QSize& size);
    static QSize defaultIconSize();

    // Оптимизация для SVG (если используется QtSvg)
    //static void setSvgRendererCacheSize(int size);

private:
    IconManager() = delete;
    ~IconManager() = delete;

    inline static QMap<QString, QIcon> s_iconCache = {};
    inline static QMap<QString, QPixmap> s_pixmapCache = {};
    inline static QMutex s_iconMutex;
    inline static QMutex s_pixmapMutex;

    // static QMap<QString, QIcon> iconCache();
    // static QMap<QString, QPixmap> pixmapCache();

    // static QMutex& iconMutex();
    // static QMutex& pixmapMutex();

    static QString generatePixmapCacheKey(const QString& path, const QSize& size);

    //static QIcon createScalableIconFromSvg(const QString& path);

    inline static QSize m_defaultIconSize = ICON_SIZE_32;
};

#define ICON_APP                    IconManager::getIcon(":/icons/app.ico")
#define ICON_EXIT                   IconManager::getIcon(":/icons/exit.svg")
#define ICON_SETTINGS               IconManager::getIcon(":/icons/settings.svg")
#define ICON_PRINT                  IconManager::getIcon(":/icons/print.svg")
#define ICON_EXPORT                 IconManager::getIcon(":/icons/export.svg")
#define ICON_REPORT                 IconManager::getIcon(":/icons/report.svg")

#define ICON_APPLY                  IconManager::getIcon(":/icons/apply.svg")
#define ICON_OPEN                   IconManager::getIcon(":/icons/open.svg")
#define ICON_SAVE                   IconManager::getIcon(":/icons/save.svg")
#define ICON_SAVE_AS                IconManager::getIcon(":/icons/save_as.svg")
#define ICON_CLOSE                  IconManager::getIcon(":/icons/close.svg")
#define ICON_CANCEL                 IconManager::getIcon(":/icons/cancel.svg")

#define ICON_ADD                    IconManager::getIcon(":/icons/add.svg")
#define ICON_ADD_BEFORE             IconManager::getIcon(":/icons/add_before.svg")
#define ICON_ADD_AFTER              IconManager::getIcon(":/icons/add_after.svg")
#define ICON_COPY                   IconManager::getIcon(":/icons/copy.svg")
#define ICON_CUT                    IconManager::getIcon(":/icons/cut.svg")
#define ICON_PASTE                  IconManager::getIcon(":/icons/paste.svg")
#define ICON_INSERT_BEFORE          IconManager::getIcon(":/icons/insert_before.svg")
#define ICON_INSERT_AFTER           IconManager::getIcon(":/icons/insert_after.svg")
#define ICON_DELETE                 IconManager::getIcon(":/icons/delete.svg")
#define ICON_OK                     IconManager::getIcon(":/icons/ok.svg")

#define ICON_LED_RED_ON             IconManager::getIcon(":/icons/led_red_on.svg")
#define ICON_LED_RED_OFF            IconManager::getIcon(":/icons/led_red_off.svg")
#define ICON_LED_GREEN_ON           IconManager::getIcon(":/icons/led_green_on.svg")
#define ICON_LED_GREEN_OFF          IconManager::getIcon(":/icons/led_green_off.svg")

#define ICON_CONNECTED              IconManager::getIcon(":/icons/connected.svg")
#define ICON_CONNECTING             IconManager::getIcon(":/icons/connecting.svg")
#define ICON_DISCONNECTED           IconManager::getIcon(":/icons/disconnected.svg")
#define ICON_LOGOUT                 IconManager::getIcon(":/icons/logout.svg")

#define ICON_ADD_TEST               IconManager::getIcon(":/icons/add_test.svg")
#define ICON_DELETE_TEST            IconManager::getIcon(":/icons/delete_test.svg")
#define ICON_LOAD                   IconManager::getIcon(":/icons/load.svg")
#define ICON_RUN                    IconManager::getIcon(":/icons/run.svg")
#define ICON_LOAD                   IconManager::getIcon(":/icons/load.svg")
#define ICON_RUN                    IconManager::getIcon(":/icons/run.svg")
#define ICON_RUN_ALL                IconManager::getIcon(":/icons/run_all.svg")
#define ICON_RUN_TRAIN              IconManager::getIcon(":/icons/run_train.svg")
#define ICON_PAUSE_RESUME           IconManager::getIcon(":/icons/pause.svg")
#define ICON_RESUME_PAUSE           IconManager::getIcon(":/icons/play.svg")
#define ICON_STOP                   IconManager::getIcon(":/icons/stop.svg")

#define ICON_SELFTEST               IconManager::getIcon(":/icons/selftest.svg")
#define ICON_CALIBRATION            IconManager::getIcon(":/icons/calibration.svg")
#define ICON_OSCILLOGRAM            IconManager::getIcon(":/icons/oscillogram.svg")
#define ICON_HISTOGRAM              IconManager::getIcon(":/icons/histogram.svg")


#define PIXMAP_APP                  IconManager::getPixmap(":/icons/app.svg")

#define PIXMAP_TEST_SEQUENCE_32x32  IconManager::getPixmap(":/icons/test_sequence_32x32.png", ICON_SIZE_32)
#define PIXMAP_TEST_32x32           IconManager::getPixmap(":/icons/test_32x32.png", ICON_SIZE_32)
#define PIXMAP_SIGNALS_32x32        IconManager::getPixmap(":/icons/signals_32x32.png", ICON_SIZE_32)
#define PIXMAP_SIGNAL_32x32         IconManager::getPixmap(":/icons/signals_32x32.png", ICON_SIZE_32)
#define PIXMAP_VARIABLES_32x32      IconManager::getPixmap(":/icons/variables_32x32.png", ICON_SIZE_32)
#define PIXMAP_VARIABLE_32x32       IconManager::getPixmap(":/icons/variables_32x32.png", ICON_SIZE_32)
#define PIXMAP_OPERATIONS_32x32     IconManager::getPixmap(":/icons/operation_32x32.png", ICON_SIZE_32)
#define PIXMAP_OPERATION_32x32      IconManager::getPixmap(":/icons/operation_32x32.png", ICON_SIZE_32)

#define PIXMAP_A_OFF                IconManager::getPixmap(":/icons/A_OFF.svg")
#define PIXMAP_A_ON                 IconManager::getPixmap(":/icons/A_ON.svg")
#define PIXMAP_K_OFF                IconManager::getPixmap(":/icons/K_OFF.svg")
#define PIXMAP_K_ON                 IconManager::getPixmap(":/icons/K_ON.svg")
#define PIXMAP_N_OFF                IconManager::getPixmap(":/icons/N_OFF.svg")
#define PIXMAP_N_ON                 IconManager::getPixmap(":/icons/N_ON.svg")

#define PIXMAP_IDLE                 IconManager::getPixmap(":/icons/idle.svg")
#define PIXMAP_MODE_READY           IconManager::getPixmap(":/icons/mode_ready.svg")
#define PIXMAP_MODE_RUNNING         IconManager::getPixmap(":/icons/mode_running.svg")
#define PIXMAP_MODE_WAITING         IconManager::getPixmap(":/icons/mode_waiting.svg")
#define PIXMAP_MODE_DONE            IconManager::getPixmap(":/icons/mode_done.svg")
#define PIXMAP_ERROR                IconManager::getPixmap(":/icons/error.svg")

#define PIXMAP_LED_RED_ON           IconManager::getPixmap(":/icons/led_red_on.svg")
#define PIXMAP_LED_RED_OFF          IconManager::getPixmap(":/icons/led_red_off.svg")
#define PIXMAP_LED_GREEN_ON         IconManager::getPixmap(":/icons/led_green_on.svg")
#define PIXMAP_LED_GREEN_OFF        IconManager::getPixmap(":/icons/led_green_off.svg")

#define PIXMAP_CONNECTED            IconManager::getPixmap(":/icons/connected.svg")
#define PIXMAP_CONNECTING           IconManager::getPixmap(":/icons/connecting.svg")
#define PIXMAP_DISCONNECTED         IconManager::getPixmap(":/icons/disconnected.svg")
#define PIXMAP_LOGOUT               IconManager::getPixmap(":/icons/logout.svg")

#endif // ICON_MANAGER_H
