#include "functions_degrees.h"

#include <climits>
#include <cmath>

#include <QtMath>

#include "Logger.h"
#include "TestRunner.h"

/*
Разрешение датчиков VIONIC
V2BJW30D50B - Resolution 200 нм
Pi*D/(65536*100) = 3.14*417000000/6553600 = 200 нм
Шкала интерполирована в 100 раз
360 градусов -> 360*60*60 = 1296000" (угловых секунд)
Разрешение 1296000"/6553600 = 0.2"

V2BJQ30D50B - Resolution 5 нм
Pi*D/(65536*4000) = 3.14*417000000/262144000 = 5 нм
Шкала интерполирована в 4000 раз
360 градусов -> 360*60*60 = 1296000" (угловых секунд)
Разрешение 1296000"/262144000 = 0.0049"

EAS - Quick Tuning - Feedback Settings - Resolution -
counts/revolution = 6553600 или 262144000 - cnt на оборот!
lines/revolution = 1638400 или 65536000
counts = 4*lines т.к. quad encoder
*/

qreal CORE_UTILS_LIB_EXPORT radiansFromDegrees(qreal degrees)
    { return degrees * M_PI / 180.0; }
qreal CORE_UTILS_LIB_EXPORT degreesFromRadians(qreal radians)
    { return radians * 180.0 / M_PI; }

double cnt_to_radians(double cnt, int interpolation) {
    return cnt * (2.0 * M_PI / (double)(65536 * interpolation));
}

double radians_to_cnt(double radians, int interpolation) {
    return radians / (2.0 * M_PI /  (double)(65536 *interpolation));
}

QString dms_to_string(int degrees, int min, double sec) {
    return QString::fromUtf8(u8"%1%2°%3′%4″")
        .arg(degrees < 0 ? "-" : "")
        .arg(degrees, 3, 10, QChar('0'))
        .arg(min, 2, 10, QChar('0'))
        .arg(sec);
}

void dms_from_string(const QString dms, double &degrees, double &minutes, double &seconds) {
    QStringList sl_degrees = dms.split("°");
    if (!sl_degrees.empty()) {
        degrees = sl_degrees.front().toDouble();
        QStringList sl_minutes = sl_degrees.back().split("′");
        if (!sl_minutes.empty()) {
            minutes = sl_minutes.front().toDouble();
            QStringList sl_seconds = sl_minutes.back().split("″");
            if (!sl_seconds.empty()) {
                seconds = sl_seconds.front().toDouble();
            }
        }
    }
}

QString radians_to_dms(double radians, int precision) {
    //static const double PI = 4*std::atan(1.);

    if (std::abs(radians) > ULONG_MAX*M_PI/180/60)
        // out of range
        return "---";

    const auto total_minutes = std::abs(radians*(60*180/M_PI));
    auto whole_minutes = static_cast<int>(total_minutes);
    auto seconds = 60 * (total_minutes - whole_minutes);

    auto secStr = QString::number(seconds, 'f', precision);
    if (secStr.startsWith("60")) {
        // it rounds up - carry to minutes
        secStr = QString::number(0.0, 'f', precision);
        whole_minutes += 1;
    } else if (secStr.startsWith('-')) {
        // negative zero - shouldn't happen
        secStr = QString::number(0.0, 'f', precision);
    }
    if (secStr.size() < 2 || secStr[1] == '.')
        // zero-fill
        secStr.prepend('0');

    return QString::fromUtf8(u8"%1%2°%3′%4″")
        .arg(radians < 0 ? "-" : "")
        .arg(whole_minutes / 60, 3, 10, QChar('0'))
        .arg(whole_minutes % 60, 2, 10, QChar('0'))
        .arg(secStr);
}

QString degrees_to_dms(double degrees, int precision) {
    double radians = qDegreesToRadians(degrees);
    return radians_to_dms(radians, precision);
}

double dms_to_radians(QString dms) {
    QStringList sl_degrees = dms.split("°");
    double degrees = 0.0;
    double minutes = 0.0;
    double seconds = 0.0;
    if (!sl_degrees.empty()) {
        degrees = sl_degrees.front().toDouble();
        QStringList sl_minutes = sl_degrees.back().split("′");
        if (!sl_minutes.empty()) {
            minutes = sl_minutes.front().toDouble();
            QStringList sl_seconds = sl_minutes.back().split("″");
            if (!sl_seconds.empty()) {
                seconds = sl_seconds.front().toDouble();
            }
        }
    }
    return dms_to_radians(degrees, minutes, seconds);
}

double dms_to_degrees(QString dms) {
    double radians = dms_to_radians(dms);
    return radians * 180.0 / M_PI;
}

double dms_to_radians(double degrees, double minutes, double seconds) {
    //    QString radians = QString::fromUtf8(u8"%1%2°%3′%4″")
    //            .arg(degrees < 0 ? "-" : "")
    //            .arg(degrees)
    //            .arg(minutes)
    //            .arg(seconds);
    if (degrees > 0.0) {
        return (degrees + (minutes / 60.0) + (seconds / 3600.0)) * M_PI / 180.0 ;
    } else {
        return (degrees - (minutes / 60.0) - (seconds / 3600.0)) * M_PI / 180.0 ;
    }
}

double dms_to_degrees(double degrees, double minutes, double seconds) {
    double radians = dms_to_radians(degrees, minutes, seconds);
    return radians*180.0/M_PI;
}

void FunctionsDegreesTest() {
    int precision = 5;
    {
        {
            int interpolation = 100;
            {
                double cnt = 1;
                //double radians_ref = 2 * M_PI / (65536*100); //0.000000958737992428;
                double radians = cnt_to_radians(cnt, interpolation);
                //ASSERT_EQUAL(radians, radians_ref);

                QString dms_ref = "000°00′00.19775″";
                QString dms = radians_to_dms(radians, precision);
                ASSERT_EQUAL(dms, dms_ref);
            }
            {
                double cnt = 6553600;
                //double radians_ref = 2.0 * M_PI * 6553600 / (65536*100);
                double radians = cnt_to_radians(cnt, interpolation);
                //ASSERT_EQUAL(radians, radians_ref);

                QString dms_ref = "360°00′00.00000″";
                QString dms = radians_to_dms(radians, precision);
                ASSERT_EQUAL(dms, dms_ref);
            }
        }
        {
            int interpolation = 4000;
            {
                double cnt = 1;
                //double radians_ref = 2 * M_PI / (65536*4000); //0.0000000239684498;
                double radians = cnt_to_radians(cnt, interpolation);
                //ASSERT_EQUAL(radians, radians_ref);

                QString dms_ref = "000°00′00.00494″";
                QString dms = radians_to_dms(radians, precision);
                ASSERT_EQUAL(dms, dms_ref);
            }
            {
                double cnt = 262144000;
                //double radians_ref = 2.0 * M_PI;
                double radians = cnt_to_radians(cnt, interpolation);
                //ASSERT_EQUAL(radians, radians_ref);

                QString dms_ref = "360°00′00.00000″";
                QString dms = radians_to_dms(radians, precision);
                ASSERT_EQUAL(dms, dms_ref);
            }
        }
    }
    {
        {
            double radians = 0.0;
            QString dms_ref = "000°00′00.00000″";
            QString dms = radians_to_dms(radians, precision);
            ASSERT_EQUAL(dms, dms_ref);
        }
        {
            double radians = M_PI;
            QString dms_ref = "180°00′00.00000″";
            QString dms = radians_to_dms(radians, precision);
            ASSERT_EQUAL(dms, dms_ref);
        }
        {
            double radians = -M_PI;
            QString dms_ref = "-180°00′00.00000″";
            QString dms = radians_to_dms(radians, precision);
            ASSERT_EQUAL(dms, dms_ref);
        }
        {
            double radians = M_PI / 2.0;
            QString dms_ref = "090°00′00.00000″";
            QString dms = radians_to_dms(radians, precision);
            ASSERT_EQUAL(dms, dms_ref);
        }
        {
            double radians = -M_PI / 2.0;
            QString dms_ref = "-090°00′00.00000″";
            QString dms = radians_to_dms(radians, precision);
            ASSERT_EQUAL(dms, dms_ref);
        }
        {
            double radians = 1.0;
            QString dms_ref = "057°17′44.80625″";
            QString dms = radians_to_dms(radians, precision);
            ASSERT_EQUAL(dms, dms_ref);
        }
        {
            double radians = -1.0;
            QString dms_ref = "-057°17′44.80625″";
            QString dms = radians_to_dms(radians, precision);
            ASSERT_EQUAL(dms, dms_ref);
        }
    }
    {
        {
            double degrees = 0.0;
            QString dms_ref = "000°00′00.00000″";
            QString dms = degrees_to_dms(degrees, precision);
            ASSERT_EQUAL(dms, dms_ref);
        }
        {
            double degrees = 180.0;
            QString dms_ref = "180°00′00.00000″";
            QString dms = degrees_to_dms(degrees, precision);
            ASSERT_EQUAL(dms, dms_ref);
        }
        {
            double degrees = -180.0;
            QString dms_ref = "-180°00′00.00000″";
            QString dms = degrees_to_dms(degrees, precision);
            ASSERT_EQUAL(dms, dms_ref);
        }
        {
            double degrees = 90.0;
            QString dms_ref = "090°00′00.00000″";
            QString dms = degrees_to_dms(degrees, precision);
            ASSERT_EQUAL(dms, dms_ref);
        }
        {
            double degrees = -90.0;
            QString dms_ref = "-090°00′00.00000″";
            QString dms = degrees_to_dms(degrees, precision);
            ASSERT_EQUAL(dms, dms_ref);
        }
        {
            double degrees = 57.29577951308;
            QString dms_ref = "057°17′44.80625″";
            QString dms = degrees_to_dms(degrees, precision);
            ASSERT_EQUAL(dms, dms_ref);
        }
        {
            double degrees = -57.29577951308;
            QString dms_ref = "-057°17′44.80625″";
            QString dms = degrees_to_dms(degrees, precision);
            ASSERT_EQUAL(dms, dms_ref);
        }
    }
    {
        {
            QString dms = "000°00′00.00000″";
            double radians_ref = 0.0;
            double radians = dms_to_radians(dms);
            ASSERT_EQUAL(radians, radians_ref);
        }
        {
            QString dms = "180°00′00.00000″";
            double radians_ref = M_PI;
            double radians = dms_to_radians(dms);
            ASSERT_EQUAL(radians, radians_ref);
        }
        {
            QString dms = "-180°00′00.00000″";
            double radians_ref = -M_PI;
            double radians = dms_to_radians(dms);
            ASSERT_EQUAL(radians, radians_ref);
        }
        {
            QString dms = "090°00′00.00000″";
            double radians_ref = M_PI / 2.0;
            double radians = dms_to_radians(dms);
            ASSERT_EQUAL(radians, radians_ref);
        }
        {
            QString dms = "-090°00′00.00000″";
            double radians_ref = -M_PI / 2.0;
            double radians = dms_to_radians(dms);
            ASSERT_EQUAL(radians, radians_ref);
        }
        {
            QString dms_ref = "057°17′44.80625″";
            double radians = dms_to_radians(dms_ref);
            QString dms = radians_to_dms(radians, precision);
            ASSERT_EQUAL(dms, dms_ref);
        }
        {
            QString dms_ref = "-057°17′44.80625″";
            double radians = dms_to_radians(dms_ref);
            QString dms = radians_to_dms(radians, precision);
            ASSERT_EQUAL(dms, dms_ref);
        }
    }
    {
        {
            QString dms = "000°00′00.00000″";
            double degrees_ref = 0.0;
            double degrees = dms_to_degrees(dms);
            ASSERT_EQUAL(degrees, degrees_ref);
        }
        {
            QString dms = "180°00′00.00000″";
            double degrees_ref = 180.0;
            double degrees = dms_to_degrees(dms);
            ASSERT_EQUAL(degrees, degrees_ref);
        }
        {
            QString dms = "-180°00′00.00000″";
            double degrees_ref = -180.0;
            double degrees = dms_to_degrees(dms);
            ASSERT_EQUAL(degrees, degrees_ref);
        }
        {
            QString dms = "090°00′00.00000″";
            double degrees_ref = 90.0;
            double degrees = dms_to_degrees(dms);
            ASSERT_EQUAL(degrees, degrees_ref);
        }
        {
            QString dms = "-090°00′00.00000″";
            double degrees_ref = -90.0;
            double degrees = dms_to_degrees(dms);
            ASSERT_EQUAL(degrees, degrees_ref);
        }
        {
            QString dms_ref = "057°17′44.80625″";
            double degrees = dms_to_degrees(dms_ref);
            QString dms = degrees_to_dms(degrees, precision);
            ASSERT_EQUAL(dms, dms_ref);
        }
        {
            QString dms_ref = "-057°17′44.80625″";
            double degrees = dms_to_degrees(dms_ref);
            QString dms = degrees_to_dms(degrees, precision);
            ASSERT_EQUAL(dms, dms_ref);
        }
    }
}
