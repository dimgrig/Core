#ifndef FILTERMIXIN_H
#define FILTERMIXIN_H

#include <map>
#include <QVector>

#include "core-utils-lib-global.h"

struct CORE_UTILS_LIB_EXPORT FilterSettings {
    std::map<QString, std::map<int, int>> filter_enable_m;
    std::map<QString, int> filter_median_m;
    std::map<QString, int> filter_mao_m;
    std::map<QString, std::tuple<double, double, double>> filter_aa_m;
};

template<typename Derived>
class FilterMixin {
public:
    virtual ~FilterMixin() = default;

    //void apply_filter_impl(double& value) = delete;

    virtual void apply_filter_impl(double& value) {
        ;
    }

    void apply_filter(double& value) {
        // Вызываем фильтр следующего уровня (если есть)
        static_cast<Derived*>(this)->apply_filter_impl(value);
    }
};


template<typename Base>
class MovingAverageFilterMixin : public Base {
public:
    void apply_filter_impl(double& value) override {
        // Вызов предыдущего фильтра в цепочке
        Base::apply_filter_impl(value);

        if (!_enable || (_window_size <= 0)) return;

        if (_filter_window.empty()) {
            //_filter_window.resize(_window_size, value);
            _filter_window.fill(value, _window_size);
        }

        // Обновляем окно
        _filter_window.push_back(value);
        if (_filter_window.size() > _window_size) {
            _filter_window.pop_front();
        }

        // Вычисляем среднее
        double sum = 0.0;
        for (double v : _filter_window) {
            sum += v;
        }
        value = sum / _filter_window.size();
    }
    void set_ma_settings(bool enable, int size) {
        _enable = enable;
        _window_size = size;
        this->ma_clear();
    }
    void ma_clear() {
        _filter_window.clear();
    }
private:
    bool _enable = false;
    int _window_size = 1;
    QVector<double> _filter_window;  // Окно для фильтрации
};

template<typename Base>
class AdaptiveMovingAverageFilterMixin : public Base {
public:
    void apply_filter_impl(double& value) override {
        // Вызов предыдущего фильтра в цепочке
        Base::apply_filter_impl(value);

        if (!_enable || (_window_size <= 0)) return;

        // 1. Обновляем буфер волатильности
        if (!_volatility_buffer.empty()) {
            double diff = std::abs(value - _prev_value);
            _volatility_buffer.push_back(diff);

            if (_volatility_buffer.size() > _volatility_window) {
                _volatility_buffer.pop_front();
            }
        }

        // 2. Рассчитываем текущую волатильность
        double sum_diff = 0.0;
        for (double diff : _volatility_buffer) {
            sum_diff += diff;
        }
        _volatility = sum_diff / _volatility_buffer.size();

        // 3. Адаптируем коэффициент сглаживания
        double adaptive_alpha = _alpha;
        if (_volatility > 0.01f) { // Порог волатильности
            adaptive_alpha = std::min((double)0.9, _alpha / _volatility);
        }

        // 4. Применяем экспоненциальное сглаживание
        if (_filter_window.empty()) {
            //_filter_window.resize(_window_size, value);
            _filter_window.fill(value, _window_size);
        } else {
            value = adaptive_alpha * value + (1 - adaptive_alpha) * _filter_window.last();
        }

        // 5. Обновляем окно фильтра
        _filter_window.push_back(value);
        if (_filter_window.size() > _window_size) {
            _filter_window.pop_front();
        }

        _prev_value = value;
    }
    void set_ama_settings(bool enable, int size, double base_alpha, int volatility_window) {
        _enable = enable;
        _window_size = size;
        _alpha = base_alpha;
        _volatility_window = volatility_window;
        this->ama_clear();
    }
    void ama_clear() {
        _volatility_buffer.clear();
    }
private:
    bool _enable = false;
    int _window_size = 1;
    QVector<double> _filter_window;  // Окно для фильтрации
    double _alpha = 0.2f;      // Базовый коэффициент сглаживания
    double _volatility = 0.0; // Текущая волатильность
    double _prev_value = 0.0; // Предыдущее значение
    int _volatility_window = 5; // Окно для расчета волатильности
    QVector<double> _volatility_buffer; // Буфер для расчета волатильности
};

template<typename Base>
class ExpMovingAverageFilterMixin : public Base {
public:
    void apply_filter_impl(double& value) override {
        // Вызов предыдущего фильтра в цепочке
        Base::apply_filter_impl(value);

        if (!_enable) return;

        double k = 0;
        if (abs(value - _exp_res) > (value / (double)_gap)) k = _fast;
        else k = _slow;

        _exp_res += (value - _exp_res) * k;
        value = _exp_res;
    }

    void set_exp_settings(bool enable, double gap, double fast, double slow) {
        _enable = enable;
        _gap = gap;
        _fast = fast;
        _slow = slow;
        _exp_res = 0.0;
    }
private:
    bool _enable = false;
    int _window_size = 1;
    double _gap = 0.0;      // Базовый коэффициент сглаживания
    double _fast = 0.0; // Текущая волатильность
    double _slow = 0.0; // Предыдущее значение
    double _exp_res = 0.0;
};

template<typename Base>
class MedianNFilterMixin : public Base {
public:
    void apply_filter_impl(double& value) override {
        // Вызов предыдущего фильтра в цепочке
        Base::apply_filter_impl(value);

        if (!_enable || (_window_size <= 0)) return;

        // Обновляем окно
        _filter_window.push_back(value);
        if (_filter_window.size() > _window_size) {
            _filter_window.pop_front();
        }

        // Вычисляем медиану
        QVector<double> sorted = _filter_window;
        std::sort(sorted.begin(), sorted.end());

        int mid = sorted.size() / 2;
        value = (sorted.size() % 2 == 1)
                    ? sorted[mid]
                    : (sorted[mid - 1] + sorted[mid]) / 2.0;
    }
    void set_median_settings(bool enable, int size) {
        _enable = enable;
        _window_size = size;
        this->median_clear();
    }
    void median_clear() {
        _filter_window.clear();
    }
private:
    bool _enable = false;
    int _window_size = 1;
    QVector<double> _filter_window;  // Окно для фильтрации
};



#endif // FILTERMIXIN_H
