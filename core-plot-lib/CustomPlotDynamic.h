#ifndef CUSTOMPLOTDYNAMIC_H
#define CUSTOMPLOTDYNAMIC_H

#include "CustomPlotBase.h"
#include "CustomPlotSettings.h"
#include "CursorManager.h"
#include "StatsManager.h"

#include <QMap>
#include <QPointer>
#include <QVector>
#include <memory>

class CustomAxisTag; // forward

class CORE_PLOT_LIB_EXPORT CustomPlotDynamic : public CustomPlotBase {
    Q_OBJECT
public:
    explicit CustomPlotDynamic(QWidget *parent = nullptr,
                               CustomPlotOrientation orientation = OrientationHorizontal);
    ~CustomPlotDynamic() override;

    // Оси
    QCPAxis* getOrCreateAxis(const QString& unit);   // возвращает левую ось, если unit совпадает с левой; иначе правую
    void setLeftAxisUnit(const QString& unit);       // устанавливает подпись левой оси
    QString leftAxisUnit() const;                    // текущая единица левой оси

    // Графики
    QCPGraph* createGraph(const QString& unit, const QString& name = QString());
    void removeGraph(QCPGraph* graph);
    void removeAllGraphs();                          // удаляет все графики и скрывает неиспользуемые правые оси

    // Доступ к графикам по имени (для совместимости)
    QCPGraph* graphByName(const QString& name) const;

    // Установка данных для графика (замена set_cp_data)
    void setGraphData(QCPGraph* graph, const QVector<double> &values, const QVector<double> &ticks);
protected slots:

private slots:
    void onAxisRangeChanged(const QCPRange &range);  // для обновления курсора/статистики

private:
    void setupUi();
    void setupConnections();
    QCPAxis* createRightAxis(const QString& unit);

    virtual void addGraphSpecificActions(QMenu *menu) override;

    QMap<QString, QCPAxis*> _unitToAxis;             // su -> ось
    QList<QCPGraph*> _graphs;                        // все графики
    QMap<QCPGraph*, QString> _graphUnit;             // график → единица измерения
    QString _leftAxisUnit;                           // текущая единица левой оси

    // Основная правая ось для курсора/статистики (всегда существует)
    QCPAxis* _activeRightAxis = nullptr;
};

#endif // CUSTOMPLOTDYNAMIC_H
