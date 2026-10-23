#include "ModbusScale.h"

#include <QStringList>

ModbusScale ModbusScale::parse(const QString& text) {
    ModbusScale s;
    const QString t = text.trimmed();
    if (t.isEmpty()) return s;

    if (t.contains(';')) {
        const QStringList parts = t.split(';', Qt::KeepEmptyParts);
        if (parts.size() == 4) {
            bool ok1, ok2, ok3, ok4;
            const double smin = parts[0].trimmed().toDouble(&ok1);
            const double smax = parts[1].trimmed().toDouble(&ok2);
            const double tmin = parts[2].trimmed().toDouble(&ok3);
            const double tmax = parts[3].trimmed().toDouble(&ok4);
            if (ok1 && ok2 && ok3 && ok4 && (smax != smin)) {
                s.mode = Range;
                s.srcMin = smin; s.srcMax = smax;
                s.dstMin = tmin; s.dstMax = tmax;
            }
        }
        return s;
    }

    bool ok = false;
    const double f = t.toDouble(&ok);
    if (ok && f > 0.0) return makeFactor(f);
    return s;
}

QString ModbusScale::toString() const {
    if (isFactor())
        return QString::number(factor);
    if (isRange())
        return QStringLiteral("%1;%2;%3;%4")
            .arg(srcMin).arg(srcMax).arg(dstMin).arg(dstMax);
    return QString();
}

double ModbusScale::applyForward(double raw) const {
    if (isFactor()) return raw * factor;
    if (isRange()) {
        const double sSpan = srcMax - srcMin;
        if (sSpan == 0.0) return dstMin;
        return dstMin + (raw - srcMin) * (dstMax - dstMin) / sSpan;
    }
    return raw;
}

double ModbusScale::applyBackward(double scaled) const {
    if (isFactor()) return scaled / factor;
    if (isRange()) {
        const double dSpan = dstMax - dstMin;
        if (dSpan == 0.0) return srcMin;
        return srcMin + (scaled - dstMin) * (srcMax - srcMin) / dSpan;
    }
    return scaled;
}
