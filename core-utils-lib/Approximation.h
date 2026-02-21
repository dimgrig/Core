#ifndef APPROXIMATION_H
#define APPROXIMATION_H

#include <vector>

#include "aqp.h"

#include "core-utils-lib-global.h"

CORE_UTILS_LIB_EXPORT void ApproximationTest();

class CORE_UTILS_LIB_EXPORT Approximation
{
public:
    explicit Approximation(int polynom_N, std::vector<double> x_v, std::vector<double> y_v);
    ~Approximation();

    void get_polynom(std::vector<double> &polynom) {polynom = _polynom;};
private:
    std::vector<double> _polynom;
};

#endif // APPROXIMATION_H
