#ifndef CALIBRATION_H
#define CALIBRATION_H

#include "math.h"

//#include "config_stm.h"
//#include "enums.h"
#include "mixins.h"

#define calibration_epsilon 0.000000001

#define MAX_POLYNOM_COEFS_N         5
#define MAX_CALIBRATION_NUMBER      10

#ifdef QT_imit
#include "Logger.h"
#include "TestRunner.h"

#include "templates.h"
#else

#endif

#include "core-utils-lib-global.h"

#ifdef QT_imit
typedef std::map<int, std::map<int, std::vector<double>>> calibration_map_t;
typedef std::map<int, std::map<int, std::pair<unsigned int, double>>> preparation_map_t;
typedef std::map<int, std::map<int, std::map<int, unsigned int>>> calibrator_map_t;
typedef std::map<int, double> calibrator_resistance_map_t;
typedef std::map<int, std::map<int, std::pair<double, double>>> calibrator_dac_map_t;

CORE_UTILS_LIB_EXPORT double calibrate(const std::vector<double> &k_v, const double value);
#endif
CORE_UTILS_LIB_EXPORT double calibrate(const double *a, const double value);
CORE_UTILS_LIB_EXPORT double calibrate(const float *a, uint8_t N, const double value);


struct CORE_UTILS_LIB_EXPORT Calibration_s : public CopyMoveBase, public not_equal<Calibration_s> {
    Calibration_s() :
        signal(0),
        range(0)
    {
        for (size_t i = 0; i < MAX_POLYNOM_COEFS_N; ++i) {
            A[i] = 0.0;
        }
        A[1] = 1.0;
    };
    Calibration_s(unsigned int s, unsigned int r) :
        signal(s),
        range(r)
    {
        for (size_t i = 0; i < MAX_POLYNOM_COEFS_N; ++i) {
            A[i] = 0.0;
        }
        A[1] = 1.0;
    };
    Calibration_s(unsigned int s, unsigned int r, double *a, size_t size) :
        signal(s),
        range(r)
    {
        for (size_t i = 0; i < MAX_POLYNOM_COEFS_N; ++i) {
            A[i] = 0.0;
        }
        if (size > MAX_POLYNOM_COEFS_N) {
            size = MAX_POLYNOM_COEFS_N;
        }
        for (size_t i = 0; i < size; ++i) {
            A[i] = *(a + i);
        }
    };

    unsigned int signal = 0;
    unsigned int range = 0;
    double A[MAX_POLYNOM_COEFS_N];

#ifdef QT_imit
    friend QDebug operator<<(QDebug os, const Calibration_s& r);
    friend std::ostream& operator<<(std::ostream &os, const Calibration_s &r);
#endif
    friend bool operator==(const Calibration_s &lhs, const Calibration_s &rhs);
};

class CORE_UTILS_LIB_EXPORT Calibration_c : public CopyMoveBase, public not_equal<Calibration_c> {
public:
    Calibration_c();
    ~Calibration_c();

    void create_calibration(Calibration_s settings);
    Calibration_s read_calibration(unsigned int signal, unsigned int range) const;
    void update_calibration(Calibration_s settings);
    void delete_calibration(unsigned int type, unsigned int range);

    Calibration_s get_calibration(unsigned int index) const;

#ifdef QT_imit
    friend QDebug operator<<(QDebug os, const Calibration_c& r);
    friend std::ostream& operator<<(std::ostream &os, const Calibration_c &r);
#endif
    friend bool operator==(const Calibration_c &lhs, const Calibration_c &rhs);
private:
    unsigned int _max_settings_nbr = MAX_CALIBRATION_NUMBER;
    unsigned int _current_settings_nbr = 0;

    Calibration_s _settings[MAX_CALIBRATION_NUMBER];
};
#endif // RMO_CALIBRATION_H
