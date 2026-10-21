#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <bitset>
#include <iomanip>
#include <iostream>
#include <vector>

#include <QApplication>
#include <QDateTime>
#include <QDialog>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QRegularExpression>
#include <QVector>

#include <core-lib/Logger.h>
#include <core-lib/templates_cout.h>
#include <core-lib/TestRunner.h>

#include "core-utils-lib-global.h"

CORE_UTILS_LIB_EXPORT void FunctionsTest();

CORE_UTILS_LIB_EXPORT std::string to_string_exact(double x, bool fixed = true, int precision = 3);
CORE_UTILS_LIB_EXPORT bool string_replace(std::string& str, const std::string& from, const std::string& to);
CORE_UTILS_LIB_EXPORT int round_up_to_value(int value, int value_to_round);
CORE_UTILS_LIB_EXPORT int round_down_to_value(int value, int value_to_round);
CORE_UTILS_LIB_EXPORT std::string trim_trailing_zeros(std::string s); // Удаляет лишние нули после запятой. Например, "3.500" → "3.5", "3.000" → "3"
CORE_UTILS_LIB_EXPORT bool startsWith(std::string mainStr, std::string toMatch);
CORE_UTILS_LIB_EXPORT std::vector<std::string> split(std::string strToSplit, char delimeter);
CORE_UTILS_LIB_EXPORT double extract_double(QString str);
CORE_UTILS_LIB_EXPORT double extract_double_scientific(QString str);
CORE_UTILS_LIB_EXPORT unsigned int extract_id(const QString str);
CORE_UTILS_LIB_EXPORT unsigned int extract_outcome_id(const QString str);
CORE_UTILS_LIB_EXPORT QString extract_name(QStringList SignalName_sl_re, const QString str);
CORE_UTILS_LIB_EXPORT QString extract_signal_type(const QString str);
CORE_UTILS_LIB_EXPORT QString extract_testType(const QStringList TestTypes_re, const QString str);
CORE_UTILS_LIB_EXPORT QString extract_standardUnit(const QString str);
CORE_UTILS_LIB_EXPORT QString extract_dimension(const QString str);
CORE_UTILS_LIB_EXPORT unsigned int extract_connectorPin(const QString str);
CORE_UTILS_LIB_EXPORT double extract_tolerance(const QString str);
CORE_UTILS_LIB_EXPORT std::string su_from_name(const std::string name);
CORE_UTILS_LIB_EXPORT double extract_scale_factor(const std::string su);

CORE_UTILS_LIB_EXPORT int extract_controller_id(const QString str);
CORE_UTILS_LIB_EXPORT int extract_sensor_id(const QString str);
CORE_UTILS_LIB_EXPORT int extract_device_id(const QString str);

CORE_UTILS_LIB_EXPORT int bit2(int x);
CORE_UTILS_LIB_EXPORT int bit3(int x);

CORE_UTILS_LIB_EXPORT std::string TRANSLATION_EN_RU(std::string EN);
CORE_UTILS_LIB_EXPORT std::string TRANSLATION_RU_EN(std::string RU);

#ifndef Q_OS_WASM
CORE_UTILS_LIB_EXPORT QString getPlatformSpecificIP();
#endif

CORE_UTILS_LIB_EXPORT void delegate_selection_paint(QPainter *painter,
                              const QStyleOptionViewItem &option,
                              QStyleOptionViewItem &opt,
                              const QModelIndex &index);

CORE_UTILS_LIB_EXPORT std::string polynom_to_string(const std::vector<double>& polynom_v, int precision = 3);
CORE_UTILS_LIB_EXPORT int determine_polynom_order(const std::string& polynom_str);
CORE_UTILS_LIB_EXPORT std::string format_polynom_for_export(const std::string& input);

CORE_UTILS_LIB_EXPORT void calculate(double &value, const QString &polynom_str);

CORE_UTILS_LIB_EXPORT void center_dialog_mainwindow(QDialog* dialog);
CORE_UTILS_LIB_EXPORT void center_dialog(QDialog *dialog);

static std::map<std::string, std::string> translations_RU_EN {
    {"кА", "kA"},
    {"кВ", "kV"},
    {"А", "A"},
    {"В", "V"},
    {"мА", "mA"},
    {"мВ", "mV"},
    {"мкА", "uA"},
    {"мкВ", "uV"},
    {"нА", "nA"},
    {"нВ", "nV"},
    {"нс", "ns"},
    {"мкс", "us"},
    {"мс", "ms"},
    {"с", "s"},

    {"ГОм", "GOhm"},
    {"МОм", "MOhm"},
    {"кОм", "kOhm"},
    {"Ом", "Ohm"},
    {"мОм", "mOhm"},
    {"мкОм", "uOhm"},
    {"нОм", "nOhm"},

    {"кСм", "kS"},
    {"См", "S"},
    {"мСм", "mS"},
    {"мкСм", "uS"},
    {"нСм", "nS"},

    {"ГГц", "GHz"},
    {"МГц", "MHz"},
    {"кГц", "kHz"},
    {"Гц", "Hz"},
    {"мГц", "mHz"},
    {"мкГц", "uHz"},
    {"нГц", "nHz"},

    {"В норме", "Passed"},
    {"Не в норме", "Failed"},
    {"Годен", "Passed"},
    {"Брак", "Failed"}
};

#define value_epsilon 0.000000001
static std::function<bool(double, double)> equal_to_double = [](double x, double y) { return std::abs(x - y) < value_epsilon; };
static std::function<bool(double, double)> not_equal_to_double = [](double x, double y) { return !(std::abs(x - y) < value_epsilon); };

#define COMPARE_DOUBLE(actual, expected) \
    QVERIFY(equal_to_double(actual, expected))

#endif // FUNCTIONS_H
