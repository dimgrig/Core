#ifndef ENERGOCYCLE80_CUSTOMPLOTSETTINGS_H
#define ENERGOCYCLE80_CUSTOMPLOTSETTINGS_H

#include <qcustomplot/qcustomplot.h>

#include <QWidget>

#include "core-plot-lib-global.h"

struct CORE_PLOT_LIB_EXPORT CustomPlotData {
public:
    CustomPlotData() :
        _v({0,0})
    {};
    CustomPlotData(CustomPlotData const &data) :
        _v(data._v)
    {};
    explicit CustomPlotData(QCPGraphData const &t) :
        _v(t)
    {};
    QCPGraphData _v;
};

//struct CustomPlotTUIData {
//public:
//    CustomPlotTUIData() :
//            u_v({0,0}),
//            i_v({0,0}),
//            t_v({0,0}),
//            t_dc_v({0,0})
//    {};
//    CustomPlotTUIData(CustomPlotTUIData const &data) :
//        u_v(data.u_v),
//        i_v(data.i_v),
//        t_v(data.t_v),
//        t_dc_v(data.t_dc_v)
//    {};
//    CustomPlotTUIData& operator=(const CustomPlotTUIData& data)
//    {
//        if (this != &data) // not a self-assignment
//        {
//            this->u_v = data.u_v;
//            this->i_v = data.i_v;
//            this->t_v = data.t_v;
//            this->t_dc_v = data.t_dc_v;
//        }
//        return *this;
//    }

//    QCPGraphData u_v;
//    QCPGraphData i_v;
//    QCPGraphData t_v;
//    QCPGraphData t_dc_v;
//};

//struct CustomPlotTData {
//public:
//    CustomPlotTData() :
//        t_v({0,0})
//    {};
//    CustomPlotTData(CustomPlotTData const &data) :
//        t_v(data.t_v)
//    {};
//    explicit CustomPlotTData(QCPGraphData const &t) :
//        t_v(t)
//    {};
//    QCPGraphData t_v;
//};

struct CORE_PLOT_LIB_EXPORT CustomPlotCalibrationData {
public:
    CustomPlotCalibrationData() :
            points_v({})/*,
            polinom_v({}),
            mode(0)*/
    {};
    CustomPlotCalibrationData(CustomPlotCalibrationData const &data) :
            points_v(data.points_v)/*,
            polinom_v(data.polinom_v),
            mode(data.mode)*/
    {};

    QVector<QCPGraphData> points_v;
//    QVector<QCPGraphData> polinom_v;
//    int mode;
};

//namespace Ui {
//    class fcp_settings;
//}

//class CustomPlotSettings : public QWidget {
//Q_OBJECT
//public:
//    explicit CustomPlotSettings(QWidget *parent = nullptr);
//    virtual ~CustomPlotSettings();
//signals:
//    void channel_changed(int channel);
//    void data_size_changed(int size);
//    void autorescale_changed(int resc);
//private:
//    Ui::fcp_settings *ui;
//};


#endif //ENERGOCYCLE80_CUSTOMPLOTSETTINGS_H
