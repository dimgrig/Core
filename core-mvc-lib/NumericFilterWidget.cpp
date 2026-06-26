#include "NumericFilterWidget.h"
#include <QRegularExpression>

NumericFilterWidget::NumericFilterWidget(QWidget *parent) : QWidget(parent) {
    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);

    _operatorCombo = new QComboBox;
    _operatorCombo->addItems({ "=", ">", "<", ">=", "<=", tr("Между") });
    mainLayout->addWidget(_operatorCombo);

    _valueSpin1 = new QDoubleSpinBox;
    _valueSpin1->setRange(-1e9, 1e9);
    _valueSpin1->setDecimals(4);
    mainLayout->addWidget(_valueSpin1);

    _rangeLabel = new QLabel(" – ");
    _rangeLabel->setVisible(false);
    mainLayout->addWidget(_rangeLabel);

    _valueSpin2 = new QDoubleSpinBox;
    _valueSpin2->setRange(-1e9, 1e9);
    _valueSpin2->setDecimals(4);
    _valueSpin2->setVisible(false);
    mainLayout->addWidget(_valueSpin2);

    connect(_operatorCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &NumericFilterWidget::onOperatorChanged);
    connect(_valueSpin1, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &NumericFilterWidget::valueChanged);
    connect(_valueSpin2, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &NumericFilterWidget::valueChanged);
    connect(_operatorCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &NumericFilterWidget::valueChanged);
}

void NumericFilterWidget::onOperatorChanged(int index) {
    bool isBetween = (index == 5); // "Между"
    _rangeLabel->setVisible(isBetween);
    _valueSpin2->setVisible(isBetween);
}

QString NumericFilterWidget::getCondition() const {
    if (_operatorCombo->currentIndex() == 5) { // "Между"
        double from = _valueSpin1->value();
        double to = _valueSpin2->value();
        if (from > to) std::swap(from, to);
        return QString("BETWEEN %1 AND %2")
            .arg(QString::number(from, 'f', 4), QString::number(to, 'f', 4));
    } else {
        QString op = _operatorCombo->currentText();
        double val = _valueSpin1->value();
        return QString("%1 %2").arg(op, QString::number(val, 'f', 4));
    }
}

void NumericFilterWidget::setCondition(const QString &condition) {
    if (condition.isEmpty()) {
        _operatorCombo->setCurrentIndex(0);
        _valueSpin1->setValue(0.0);
        _valueSpin2->setValue(0.0);
        return;
    }

    // Пытаемся разобрать BETWEEN
    QRegularExpression betweenRe("^BETWEEN\\s+([0-9.-]+)\\s+AND\\s+([0-9.-]+)$",
                                 QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = betweenRe.match(condition.trimmed());
    if (match.hasMatch()) {
        _operatorCombo->setCurrentIndex(5);
        _valueSpin1->setValue(match.captured(1).toDouble());
        _valueSpin2->setValue(match.captured(2).toDouble());
        return;
    }

    // Пытаемся разобрать одиночный оператор (>=, <=, >, <, =) и число
    QRegularExpression singleRe("^(>=|<=|>|<|=)\\s+([0-9.-]+)$");
    match = singleRe.match(condition.trimmed());
    if (match.hasMatch()) {
        QString op = match.captured(1);
        double val = match.captured(2).toDouble();

        // Находим индекс оператора в комбобоксе
        int idx = _operatorCombo->findText(op);
        if (idx == -1) idx = 0;
        _operatorCombo->setCurrentIndex(idx);
        _valueSpin1->setValue(val);
        // переключим в одиночный режим (не "Между")
        // onOperatorChanged будет вызван автоматически
        return;
    }

    // Не удалось разобрать — сброс
    _operatorCombo->setCurrentIndex(0);
    _valueSpin1->setValue(0.0);
    _valueSpin2->setValue(0.0);
}
