#ifndef MODBUSSCALE_H
#define MODBUSSCALE_H

#include <QString>
#include <QMetaType>

#include "core-modbus-lib-global.h"

// Описание масштабирования значения регистра:
//   None   — не задано
//   Factor — умножение/деление на коэффициент ("0.1", "0.01", ...) — обратная совместимость
//   Range  — линейная интерполяция диапазонов MasterSCADA 4D: "SrcMin;SrcMax;DstMin;DstMax"
struct CORE_MODBUS_LIB_EXPORT ModbusScale {
    enum Mode { None = 0, Factor = 1, Range = 2 };

    Mode   mode   = None;
    double factor = 0.0;
    double srcMin = 0.0, srcMax = 0.0;
    double dstMin = 0.0, dstMax = 0.0;

    // Неявный конструктор от double — для обратной совместимости со старым кодом,
    // где параметр был double scale = 0.0.
    ModbusScale(double f = 0.0) {
        if (f > 0.0) { mode = Factor; factor = f; }
    }

    bool isNone()   const { return mode == None;   }
    bool isFactor() const { return mode == Factor; }
    bool isRange()  const { return mode == Range;  }

    static ModbusScale makeFactor(double f) {
        ModbusScale s;
        if (f > 0.0) { s.mode = Factor; s.factor = f; }
        return s;
    }
    static ModbusScale makeRange(double smn, double smx, double tmn, double tmx) {
        ModbusScale s;
        s.mode = Range;
        s.srcMin = smn; s.srcMax = smx;
        s.dstMin = tmn; s.dstMax = tmx;
        return s;
    }

    // "0.1" -> Factor; "0;100;0;1" -> Range; "" -> None
    static ModbusScale parse(const QString& text);

    // "0.1" для Factor, "0;100;0;1" для Range, "" для None
    QString toString() const;

    // raw -> значение (при чтении из устройства)
    double applyForward(double raw) const;
    // значение -> raw (при записи в устройство)
    double applyBackward(double scaled) const;
};

Q_DECLARE_METATYPE(ModbusScale)

#endif // MODBUSSCALE_H
