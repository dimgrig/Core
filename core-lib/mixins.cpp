#include "mixins.h"

//void TimeRangesMixin::set_last_day(QDateTimeEdit* edit) {
//    QDateTime last_day = QDateTime::currentDateTime();
//    edit->setDateTime(last_day.addDays(-1));
//}

//void TimeRangesMixin::set_last_half_day(QDateTimeEdit* edit) {
//    QDateTime last_half_day = QDateTime::currentDateTime();
//    edit->setDateTime(last_half_day.addSecs(-43200));
//}

//void TimeRangesMixin::set_last_hour(QDateTimeEdit* edit) {
//    QDateTime last_hour = QDateTime::currentDateTime();
//    edit->setDateTime(last_hour.addSecs(-3600));
//}




WidgetResizeMixin::WidgetResizeMixin(QWidget *parent) :
        QWidget(parent)
{

}

WidgetResizeMixin::~WidgetResizeMixin() {

}

void WidgetResizeMixin::resizeEvent(QResizeEvent *event) {
    emit resize_event(0, 0);
    QWidget::resizeEvent(event);
}

void WidgetResizeMixin::paintEvent(QPaintEvent *event) {

    QWidget::paintEvent(event);
}
