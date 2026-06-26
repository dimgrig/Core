#ifndef CUSTOMPLOTBASE_H
#define CUSTOMPLOTBASE_H

#include <QWidget>

#include "qcustomplot.h"

#include "CoreConfig.h"

#include "CustomAxisTag.h"

#include "CursorManager.h"
#include "StatsManager.h"

#include "core-plot-lib-global.h"

static std::vector<QString> PEN_COLOR_v {G_COLOR, R_COLOR, B_COLOR};
static std::vector<QString> SELECTION_PEN_COLOR_v {DG_COLOR, DR_COLOR, DB_COLOR};
static std::vector<QString> AXIS_TAG_PEN_COLOR_v {LG_COLOR, _LR_COLOR, LB_COLOR};

static std::vector<QString> PEN_COLOR_EXT_v {
    G_COLOR,
    R_COLOR,
    B_COLOR,
    COLOR_Yellow,
    COLOR_Orange,
    COLOR_Fuchsia,
    COLOR_Aqua,
    COLOR_Gray,
    COLOR_SandyBrown,
    COLOR_PaleGreen,
    COLOR_DeepPink,
    COLOR_Navy,
};

struct GraphStyle {
    QColor color;
    int width = 1;
    QCPGraph::LineStyle lineStyle = QCPGraph::LineStyle::lsLine;
    Qt::PenStyle penStyle = Qt::PenStyle::SolidLine;
    bool operator==(const GraphStyle &other) const {
        return ((this->color == other.color) &&
                (this->width == other.width) &&
                (this->lineStyle == other.lineStyle) &&
                (this->penStyle == other.penStyle));
    }
};

struct ThresholdLine {
    QCPItemStraightLine* line = nullptr;
    QCPItemText* label = nullptr;
    QCPAxis* axis = nullptr;   // ось Y, к которой привязана
    double value = 0.0;
    QColor color = Qt::red;
    Qt::PenStyle style = Qt::DashLine;
    int width = 1;
    QString text;
    // Для сериализации
    void saveSettings(QSettings& settings, int index) const;
    bool loadSettings(QSettings& settings, int index, const QList<QCPAxis*>& availableAxes);
};

struct VerticalCursor {
    QCPItemStraightLine* line = nullptr;
    QCPItemText* label = nullptr;
    double xValue = 0.0;
    QColor color = Qt::blue;
    int width = 1;
    Qt::PenStyle style = Qt::DashDotLine;
    bool visible = true;
    QString labelText; // автоматически формируется из xValue

    void saveSettings(QSettings& settings, int index) const;
    bool loadSettings(QSettings& settings, int index);
};

class CORE_PLOT_LIB_EXPORT CustomPlotBase : public QCustomPlot {
Q_OBJECT
public:
    explicit CustomPlotBase(const QString &objectName, QWidget *parent = nullptr);
    ~CustomPlotBase() override;

    enum CustomPlotOrientation {
        OrientationHorizontal = 0,
        OrientationVertical = 1
    };

    virtual void export_data(QTextStream &stream);
    virtual void sync_selection();

    void saveAllSettings();
    void loadAllSettings();

    bool get_autorescale() const {return this->_autorescale;}
    void set_autorescale(bool autorescale) {this->_autorescale = autorescale;}
    void set_scale_home();

    bool get_legend_state() const {return this->_legend_state;}
    void set_legend_state(bool state);

    int get_width() const {return _width;}

    void setCursorMode(CursorManager::Mode mode);
    CursorManager::Mode cursorMode() const;

    void setStatsVisible(bool visible);
    bool statsVisible() const;
    void setStatsOnlyVisibleRange(bool onlyVisible);
    bool statsOnlyVisibleRange() const;
    void updateStats(); // принудительное обновление
    void updateStatsIfVisible();

    void syncLegendVisibility();

    void addThresholdLine(QCPAxis* axis, double value,
                          const QColor& color = Qt::red,
                          Qt::PenStyle style = Qt::DashLine,
                          int width = 1,
                          const QString& text = QString());
    void removeThresholdLine(int index);
    void clearThresholdLines();
    void updateThresholdLines();
    void saveThresholdLines();
    void loadThresholdLines(const QList<QCPAxis*>& availableAxes);

    const QList<VerticalCursor>& verticalCursors() const { return _verticalCursors; }
    void addVerticalCursor(double xValue, const QColor& color = Qt::blue, int width = 1);
    void removeVerticalCursor(int index);
    void clearVerticalCursors();
    void updateCursorLabels();
    void saveVerticalCursors();
    void loadVerticalCursors();

    void rescale(int index = 0);
    void autoscale_all();
protected slots:
    void move_legend(int pos);

    void export_csv();
    void export_png();

    virtual void mouse_press();
    virtual void mouse_wheel();
    virtual void selection_changed();

    void contextMenuRequested(QPoint pos);

    // Статические умолчания
    static QColor defaultGraphColor(int index);
    static int defaultGraphWidth() { return 1; }
    static Qt::PenStyle defaultGraphPenStyle() { return Qt::PenStyle::SolidLine; }
    static QCPGraph::LineStyle defaultGraphLineStyle() { return QCPGraph::LineStyle::lsLine; }

    // Применить стиль (без сохранения)
    void applyGraphStyle(QCPGraph* graph, const GraphStyle& style);

    // Применить сохранённый (или умолчание) для заданного ключа
    void applySavedStyle(QCPGraph* graph, const QString& styleKey);

    // Сохранить стиль
    void saveStyle(const QString& styleKey, const GraphStyle& style);
    // Загрузить стиль (или умолчание)
    GraphStyle loadStyle(const QString& styleKey, int colorIndex) const;
signals:

public slots:
    void showStyleDialog(QCPGraph* graph);
private slots:

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

    std::unique_ptr<CursorManager> _cursorManager;
    std::unique_ptr<StatsManager> _statsManager;

    QList<QCPAxis*> _rightAxes;
    QList<ThresholdLine> _thresholdLines;
    QList<VerticalCursor> _verticalCursors;
private:
    virtual void addGraphSpecificActions(QMenu *menu) {Q_UNUSED(menu)}
    void addScaleActions(QMenu *menu);
    void addAxisActions(QMenu *menu);
    void addThresholdLinesActions(QMenu *menu);
    void addVerticalCursorActions(QMenu* menu);

    void showAxisRangeDialog(QCPAxis* axis);

    void showAddThresholdDialog();
    void showEditThresholdDialog(int index);

    void showAddVerticalCursorDialog();
    void showEditVerticalCursorDialog(int index);

    bool _autorescale = true;
    bool _legend_state = true;
    bool _zoomRectMode = false;

    int _width = 1;
};

#endif // CUSTOMPLOTBASE_H
