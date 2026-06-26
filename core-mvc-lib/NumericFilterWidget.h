#ifndef NUMERICFILTERWIDGET_H
#define NUMERICFILTERWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QHBoxLayout>

#include "core-mvc-lib-global.h"

class CORE_MVC_LIB_EXPORT NumericFilterWidget : public QWidget {
    Q_OBJECT
public:
    explicit NumericFilterWidget(QWidget *parent = nullptr);
    QString getCondition() const;
    void setCondition(const QString &condition);   // восстановить из сохранённой строки

signals:
    void valueChanged();

private slots:
    void onOperatorChanged(int index);

private:
    QComboBox *_operatorCombo;
    QDoubleSpinBox *_valueSpin1;
    QDoubleSpinBox *_valueSpin2;
    QLabel *_rangeLabel;
};
#endif
