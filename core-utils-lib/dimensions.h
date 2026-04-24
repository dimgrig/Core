#ifndef DIMENSIONS_H
#define DIMENSIONS_H

#include <iostream>
#include <map>
#include <vector>

#include <QStringList>

static std::pair<double, double> n_pair = {1, 1000000000};
static std::pair<double, double> u_pair = {1, 1000000};
static std::pair<double, double> m_pair = {1, 1000};
static std::pair<double, double> k_pair = {1000, 1};
static std::pair<double, double> M_pair = {1000000, 1};
static std::pair<double, double> G_pair = {1000000000, 1};

static std::map<std::string, std::pair<double, double>> dimension_m = {
    {"n", n_pair},
    {"u", u_pair},
    {"m", m_pair},
    {"k", k_pair},
    {"M", M_pair},
    {"G", G_pair},
    {"н", n_pair},
    {"мк", u_pair},
    {"м", m_pair},
    {"к", k_pair},
    {"М", M_pair},
    {"Г", G_pair}
};

static std::vector<std::string> su_v = {"с", "А", "В", "Ом", "См", "Гц", "c", "A", "V", "Ohm", "S", "Hz"};
static std::vector<std::string> dimension_v = {"n", "u", "m", "k", "M", "G", "н", "мк", "м", "к", "М", "Г"};
static std::vector<int> dimension_erase_v = {1, 1, 1, 1, 1, 1, 2, 4, 2, 2, 2, 2};

static std::vector<std::string> dimension_up = {"к", "М", "Г"};
static std::vector<std::string> dimension_down = {"м", "мк", "н"};

static std::map<std::string, QStringList> dimension_metric_prefix_m = {
    {"А", {"А", "мА", "мкА", "нА"}},
    {"В", {"В", "мВ"}},
    {"Ом", {"Ом", "мОм"}},
    {"См", {"См", "мСм"}},
    {"Гц", {"Гц", "мГц, мкГц"}}
};

static std::map<std::string, std::string> signal_dimension_m = {
    {"I", "А"},
    {"U", "В",},
    {"R", "Ом"},
    {"S", "См"},
    {"Hz", "Гц"}
};


#endif // DIMENSIONS_H
