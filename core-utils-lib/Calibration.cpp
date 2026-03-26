#include "Calibration.h"

#ifdef QT_imit

double calibrate(const std::vector<double> &k_v, const double value) {
    double res = 0.0;
    if (k_v.empty()) {
        res = value;
    }
    for (size_t i = 0; i < k_v.size(); ++i) {
        res += k_v.at(i) * std::pow(value, i);
    }
    return res;
}
#endif

double calibrate(const double* a, const double value) {
    double res = 0.0;
    for (size_t i = 0; i < MAX_POLYNOM_COEFS_N; ++i) {
        res += *(a + i) * std::pow(value, i);
    }
    return res;
}

double calibrate(const float* a, uint8_t N, const double value) {
	double res = 0.0;
    for (size_t i = 0; i < N; ++i) {
        res += *(a + i) * std::pow(value, i);
    }
    return res;
}

/***********************************************************************/

#ifdef QT_imit
QDebug operator<<(QDebug os, const Calibration_s& r) {
    os << r.signal << r.range;
    for (unsigned int i = 0; i < MAX_POLYNOM_COEFS_N; ++i) {
        os << r.A[i];
    }
    return os;
}

std::ostream& operator<<(std::ostream &os, const Calibration_s &r) {
    os << r.signal << r.range;
    for (unsigned int i = 0; i < MAX_POLYNOM_COEFS_N; ++i) {
        os << r.A[i];
    }
    return os;
}

#endif

bool operator==(const Calibration_s &lhs, const Calibration_s &rhs) {
    for (unsigned int i = 0; i < MAX_POLYNOM_COEFS_N; ++i) {
        if (std::abs(lhs.A[i] - rhs.A[i]) < calibration_epsilon) {
            continue;
        }
        if (lhs.A[i] != rhs.A[i]) {
            return false;
        }
    }
    return ((lhs.signal == rhs.signal) &&
            (lhs.range == rhs.range));
}

/***********************************************************************/

Calibration_c::Calibration_c() {

}

Calibration_c::~Calibration_c() {

}

void Calibration_c::create_calibration(Calibration_s settings) {
    if ((_current_settings_nbr + 1) < _max_settings_nbr) {
        _settings[_current_settings_nbr] = settings;
        _current_settings_nbr++;
    }
}

Calibration_s Calibration_c::read_calibration(unsigned int signal, unsigned int range) const {
    Calibration_s res;
    for (unsigned int i = 0; i < _current_settings_nbr; ++i) {
        if ((_settings[i].signal == signal) && (_settings[i].range == range)) {
            res = _settings[i];
            break;
        }
    }
    return res;
}

void Calibration_c::update_calibration(Calibration_s settings) {
    bool found = false;
    for (unsigned int i = 0; i < _current_settings_nbr; ++i) {
        if ((_settings[i].signal == settings.signal) && (_settings[i].range == settings.range)) {
            _settings[i] = settings;
            found = true;
            break;
        }
    }
    if (!found) {
        this->create_calibration(settings);
    }
}

void Calibration_c::delete_calibration(unsigned int signal, unsigned int range) {
    int index_to_delete = -1;
    for (unsigned int i = 0; i < _current_settings_nbr; ++i) {
        if ((_settings[i].signal == signal) && (_settings[i].range == range)) {
            index_to_delete = i;
        }
    }
    if (index_to_delete != -1) {
        for (unsigned int i = index_to_delete; i < _current_settings_nbr - 1; ++i) {
            _settings[i] = _settings[i+1];
        }
        _current_settings_nbr--;
    }
}

Calibration_s Calibration_c::get_calibration(unsigned int index) const {
	Calibration_s res;
    if (index < _current_settings_nbr) {
        res = _settings[index];
    }
    return res;
}

#ifdef QT_imit
QDebug operator<<(QDebug os, const Calibration_c& r) {
    for (unsigned int i = 0; i < r._current_settings_nbr; ++i) {
        os << r._settings[i];
    }
    return os;
}

std::ostream& operator<<(std::ostream &os, const Calibration_c &r) {
    for (unsigned int i = 0; i < r._current_settings_nbr; ++i) {
        os << r._settings[i];
    }
    return os;
}
#endif

bool operator==(const Calibration_c &lhs, const Calibration_c &rhs) {
    if (lhs._max_settings_nbr != rhs._max_settings_nbr ||
        lhs._current_settings_nbr != rhs._current_settings_nbr)
        return false;

    // Сравниваем только актуальные элементы (до _current_settings_nbr)
    for (unsigned int i = 0; i < lhs._current_settings_nbr; ++i) {
        if (!(lhs._settings[i] == rhs._settings[i]))
            return false;
    }
    return true;
}
