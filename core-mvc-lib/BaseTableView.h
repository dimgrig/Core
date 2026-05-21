#ifndef BASETABLEVIEW_H
#define BASETABLEVIEW_H

#include <type_traits>

#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QShortcut>

#include <QTableView>
#include <QStandardItem>
#include <QMenu>
#include <QAction>
#include <QDebug>
#include <QTimer>

#include "core-mvc-lib-global.h"

enum TableType {
    //saks
    TableType_MainTable,
    TableType_Journal,
    TableType_JournalCategories,
    TableType_Devices,
    TableType_DeviceChannels,
    TableType_Measurements,
    TableType_CalibrationRanges,
    TableType_Calibration,
    TableType_ModbusReg,

    //upkd
    TableType_StaticPressureCalibration,
    TableType_StaticPressureTolerance,
    TableType_StaticPressure,
    TableType_StaticPressureResult,

    TableType_Pulse,
    TableType_PulseResult,

    TableType_VerificationTable,
    TableType_BPMTable,
    TableType_MITable,
    TableType_TestsTable,
    TableType_UsersTable,

    //himik
    TableType_Controllers,
    TableType_Sensors,
    TableType_Clients,
    TableType_Models,
    TableType_Users,

    //rv-led
    TableType_Test,
    TableType_TestResult,
    TableType_TestsResult
};

class BaseTableView;

CORE_MVC_LIB_EXPORT void get_html_header_stream(QStringList sl, QStringList sl_class, BaseTableView* tw, QTextStream &stream);
CORE_MVC_LIB_EXPORT void get_html_row_stream(QStringList sl, BaseTableView* tw, QTextStream &stream);
CORE_MVC_LIB_EXPORT void get_csv_row_stream(QStringList sl, BaseTableView* tw, QTextStream &stream);

// template<template<class> class TW, class T, class BT> //T - TestFet2; BT - TestFet //*f - possibly no need cause delimeter get_test and get_test_result
// void template_get_test_card_html(QString filename, TW<T>* tw, QStringList (BT::*f)() const,
//               QStringList header_sl, QStringList header_class_sl,
//               QTextStream &stream, int row) {
//     Q_UNUSED(row);
//     TestSequence test_results;
//     ATMLReader::TestResults<T>(filename.toStdString(), test_results);

//     TestSequence tests;
//     //qCDebug(logDebug()) << test_results.get_test_card_parameters().get_filename();
//     ATMLReader::TestDescription<T>(test_results.get_test_card_parameters().get_filename(), tests);

//     stream << QString("<h3>%1</h3>").arg("Тестовая карта");
//     stream << QString("<h4>%1</h4>").arg(test_results.get_test_card_parameters().get_print_row().c_str());
//     stream << "<table border=1 cellspacing=0 cellpadding=2>";
//     get_html_header_stream(header_sl, header_class_sl, tw, stream);
//     for (const auto & _test : tests) {
//         std::shared_ptr<T> test = std::dynamic_pointer_cast<T>(_test);
//         get_html_row_stream((test.get()->*f)(), tw, stream); //std::bind(f, test) //test->get_test_sl()
//     }
//     stream << "</table>";
// }

// template<template<class> class TW, class T, class BT> //T - TestFet2; BT - TestFet
// void template_get_test_result_html(QString filename, TW<T>* tw, QStringList (BT::*f)() const,
//               QStringList header_sl, QStringList header_class_sl,
//               QTextStream &stream, int row) {
//     Q_UNUSED(row);
//     TestSequence test_results;
//     ATMLReader::TestResults<T>(filename.toStdString(), test_results);

//     //stream << QString("<h3>%1</h3>").arg("");
//     //stream << QString("<h4>%1</h4>").arg("");
//     stream << "<table border=1 cellspacing=0 cellpadding=2>";
//     get_html_header_stream(header_sl, header_class_sl, tw, stream);
//     for (const auto & _test : test_results) {
//         std::shared_ptr<T> test = std::dynamic_pointer_cast<T>(_test);
//         get_html_row_stream((test.get()->*f)(), tw, stream); //std::bind(f, test) //test->get_test_sl()
//     }
//     stream << "</table>";
// }

// template<template<class> class TW, class T, class BT>
// void template_get_test_card_csv(QString filename, TW<T>* tw, QStringList (BT::*f)() const,
//               QStringList header_sl,
//               QTextStream &stream, int row) {
//     Q_UNUSED(row);
//     TestSequence test_results;
//     ATMLReader::TestResults<T>(filename.toStdString(), test_results);

//     TestSequence tests;
//     //qCDebug(logDebug()) << test_results.get_test_card_parameters().get_filename();
//     ATMLReader::TestDescription<T>(test_results.get_test_card_parameters().get_filename(), tests);

//     get_csv_row_stream(header_sl, tw, stream);
//     for (const auto & _test : tests) {
//         std::shared_ptr<T> test = std::dynamic_pointer_cast<T>(_test);
//         get_csv_row_stream((test.get()->*f)(), tw, stream);
//     }
//     stream << "\n";
// }

// template<template<class> class TW, class T, class BT>
// void template_get_test_result_csv(QString filename, TW<T>* tw, QStringList (BT::*f)() const,
//               QStringList header_sl,
//               QTextStream &stream, int row) {
//     Q_UNUSED(row);
//     TestSequence test_results;
//     ATMLReader::TestResults<T>(filename.toStdString(), test_results);

//     get_csv_row_stream(header_sl, tw, stream);
//     for (const auto & _test : test_results) {
//         std::shared_ptr<T> test = std::dynamic_pointer_cast<T>(_test);
//         get_csv_row_stream((test.get()->*f)(), tw, stream);
//     }
//     stream << "\n";
// }

class CORE_MVC_LIB_EXPORT BaseTableView : public QTableView {
Q_OBJECT
public:
    BaseTableView(QWidget *parent = nullptr);
    ~BaseTableView();

    virtual TableType get_table_type() = 0;
    virtual void set_table_title() = 0;
    virtual void set_table_subtitle() = 0;
    // virtual QStringList get_test_column_html() {return {};}; //non zero cause TestResult and TestsResult not override this
    // virtual QStringList get_test_column_class_html() {return {};}; //non zero cause TestResult and TestsResult not override this
    virtual void get_points_html(QTextStream &stream, int row) {Q_UNUSED(row); Q_UNUSED(stream)}; //non zero cause Test and TestResult not override this
    virtual void get_points_csv(QTextStream &stream, int row) {Q_UNUSED(row); Q_UNUSED(stream)}; //non zero cause Test and TestResult not override this

    void restore_settings();

    void setModel(QAbstractItemModel *model) override;
signals:

public slots:
    void slotHeaderCustomMenuRequested(QPoint pos);
    void copy_to_clipboard(QItemSelectionModel *select);
    QString get_header();
    QString get_text(int row = -1);
    void view_repaint();
    void view_repaint(int row, int col);
    void set_row_height(int row, int height);
    void set_col_width(int col, int width);

    virtual void slot_print();
    virtual void slot_export_html();
    virtual void slot_export_pdf();
    virtual void slot_export_csv();
    virtual void slot_export_txt();


    void get_html_stream(QTextStream &stream);
    void get_csv_stream(QTextStream &stream);
protected slots:
    void hide_column();
    void show_column();
protected:
    void closeEvent(QCloseEvent *event) override;

    void add_print_export_actions(QMenu *menu);

    void get_html_head(QTextStream &stream);
    void get_html_table(TableType table_type, QString table_title, QString table_subtitle, QTextStream &stream, int header = 1, int row = -1);
    void get_html(QTextStream &stream);

    bool check_empty() const;
    QString file_dialog(QString file_suffix);
    void export_options_dialog(bool &measurements_export);

    QString _table_title;
    QString _table_subtitle;
private:

};

#endif // BASETABLEVIEW_H
