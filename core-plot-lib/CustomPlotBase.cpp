#include "CustomPlotBase.h"

CustomPlotBase::CustomPlotBase(QWidget *parent) :
    QCustomPlot(parent)
{
    this->setWindowFlags(Qt::Window);

    this->setMinimumSize(QSize(200, 200));
    QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    this->setSizePolicy(sizePolicy);
    this->setPlottingHint(QCP::PlottingHint::phCacheLabels, false);

    //this->axisRect()->setMargins(QMargins(100, 100, 100, 100));

    if (this->axisRect()->axis(QCPAxis::atRight, 1)) {
        this->axisRect()->axis(QCPAxis::atRight, 0)->setPadding(75); //must be for axis tag
    }

    connect(this, &CustomPlotBase::selectionChangedByUser, this, &CustomPlotBase::selection_changed);

    // connect slots that takes care that when an axis is selected, only that direction can be dragged and zoomed:
    connect(this, &CustomPlotBase::mousePress, this, &CustomPlotBase::mouse_press);
    connect(this, &CustomPlotBase::mouseWheel, this, &CustomPlotBase::mouse_wheel);

    connect(this->xAxis, SIGNAL(rangeChanged(QCPRange)), this->xAxis2, SLOT(setRange(QCPRange)));
    connect(this->yAxis, SIGNAL(rangeChanged(QCPRange)), this->yAxis2, SLOT(setRange(QCPRange)));
}

CustomPlotBase::~CustomPlotBase() {

}

void CustomPlotBase::move_legend(int pos) {
    this->axisRect()->insetLayout()->setInsetAlignment(0, (Qt::Alignment)pos);
    this->replot();
}

void CustomPlotBase::mouse_press() {
    // if an axis is selected, only allow the direction of that axis to be dragged
    // if no axis is selected, both directions may be dragged
    if (this->xAxis->selectedParts().testFlag(QCPAxis::spAxis)) {
        this->axisRect()->setRangeDragAxes(this->xAxis, this->yAxis);
        this->axisRect()->setRangeDrag(this->xAxis->orientation());
    } else if (this->xAxis2->selectedParts().testFlag(QCPAxis::spAxis)) {
        this->axisRect()->setRangeDragAxes(this->xAxis2, this->yAxis);
        this->axisRect()->setRangeDrag(this->yAxis->orientation());
    } else if (this->yAxis->selectedParts().testFlag(QCPAxis::spAxis)) {
        this->axisRect()->setRangeDragAxes(this->xAxis, this->yAxis);
        this->axisRect()->setRangeDrag(this->yAxis->orientation());
    } else if (this->yAxis2->selectedParts().testFlag(QCPAxis::spAxis)) {
        this->axisRect()->setRangeDragAxes(this->xAxis, this->yAxis2);
        this->axisRect()->setRangeDrag(this->yAxis2->orientation());
    } else if (this->axisRect()->axis(QCPAxis::atRight, 1)) {
        if (this->axisRect()->axis(QCPAxis::atRight, 1)->selectedParts().testFlag(QCPAxis::spAxis)) {
        this->axisRect()->setRangeDragAxes(this->xAxis, this->axisRect()->axis(QCPAxis::atRight, 1));
        this->axisRect()->setRangeDrag(this->axisRect()->axis(QCPAxis::atRight, 1)->orientation());
//    } else if (this->axisRect()->axis(QCPAxis::atRight, 2)->selectedParts().testFlag(QCPAxis::spAxis)) {
//        this->axisRect()->setRangeDragAxes(this->xAxis, this->axisRect()->axis(QCPAxis::atRight, 2));
//        this->axisRect()->setRangeDrag(this->axisRect()->axis(QCPAxis::atRight, 2)->orientation());
//    } else if (this->axisRect()->axis(QCPAxis::atRight, 3)->selectedParts().testFlag(QCPAxis::spAxis)) {
//        this->axisRect()->setRangeDragAxes(this->xAxis, this->axisRect()->axis(QCPAxis::atRight, 3));
//        this->axisRect()->setRangeDrag(this->axisRect()->axis(QCPAxis::atRight, 3)->orientation());
        } else {
            this->axisRect()->setRangeDragAxes(this->xAxis, this->yAxis);
            this->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
        }
    } else {
        this->axisRect()->setRangeDragAxes(this->xAxis, this->yAxis);
        this->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
    }
}

void CustomPlotBase::mouse_wheel()
{
    // if an axis is selected, only allow the direction of that axis to be zoomed
    // if no axis is selected, both directions may be zoomed

    if (this->xAxis->selectedParts().testFlag(QCPAxis::spAxis)) {
        this->axisRect()->setRangeZoomAxes(this->xAxis, this->yAxis);
        this->axisRect()->setRangeZoom(this->xAxis->orientation());
    } else if (this->xAxis2->selectedParts().testFlag(QCPAxis::spAxis)) {
        this->axisRect()->setRangeDragAxes(this->xAxis2, this->yAxis);
        this->axisRect()->setRangeDrag(this->yAxis->orientation());
    } else if (this->yAxis->selectedParts().testFlag(QCPAxis::spAxis)) {
        this->axisRect()->setRangeZoomAxes(this->xAxis, this->yAxis);
        this->axisRect()->setRangeZoom(this->yAxis->orientation());
    } else if (this->yAxis2->selectedParts().testFlag(QCPAxis::spAxis)) {
        this->axisRect()->setRangeZoomAxes(this->xAxis, this->yAxis2);
        this->axisRect()->setRangeZoom(this->yAxis2->orientation());
    } else if (this->axisRect()->axis(QCPAxis::atRight, 1)) {
        if (this->axisRect()->axis(QCPAxis::atRight, 1)->selectedParts().testFlag(QCPAxis::spAxis)) {
        this->axisRect()->setRangeZoomAxes(this->xAxis, this->axisRect()->axis(QCPAxis::atRight, 1));
        this->axisRect()->setRangeZoom(this->axisRect()->axis(QCPAxis::atRight, 1)->orientation());
//    } else if (this->axisRect()->axis(QCPAxis::atRight, 2)->selectedParts().testFlag(QCPAxis::spAxis)) {
//        this->axisRect()->setRangeZoomAxes(this->xAxis, this->axisRect()->axis(QCPAxis::atRight, 2));
//        this->axisRect()->setRangeZoom(this->axisRect()->axis(QCPAxis::atRight, 2)->orientation());
//    } else if (this->axisRect()->axis(QCPAxis::atRight, 3)->selectedParts().testFlag(QCPAxis::spAxis)) {
//        this->axisRect()->setRangeZoomAxes(this->xAxis, this->axisRect()->axis(QCPAxis::atRight, 3));
//        this->axisRect()->setRangeZoom(this->axisRect()->axis(QCPAxis::atRight, 3)->orientation());
        } else {
            this->axisRect()->setRangeZoomAxes(this->xAxis, this->yAxis);
            this->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
        }
    } else {
        this->axisRect()->setRangeZoomAxes(this->xAxis, this->yAxis);
        this->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
    }
}

void CustomPlotBase::selection_changed() {
    // make top and bottom axes be selected synchronously, and handle axis and tick labels as one selectable object:
    if (this->xAxis->selectedParts().testFlag(QCPAxis::spAxis) ||
        this->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
        this->xAxis2->selectedParts().testFlag(QCPAxis::spAxis) ||
        this->xAxis2->selectedParts().testFlag(QCPAxis::spTickLabels)) {
        this->xAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
        this->xAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    }

    // make left and right axes be selected synchronously, and handle axis and tick labels as one selectable object:
    if (this->yAxis->selectedParts().testFlag(QCPAxis::spAxis) ||
        this->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
        this->yAxis2->selectedParts().testFlag(QCPAxis::spAxis) ||
        this->yAxis2->selectedParts().testFlag(QCPAxis::spTickLabels)    ) {
        this->yAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
        this->yAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    }

    if (this->axisRect()->axis(QCPAxis::atRight, 1)) {
        if (this->axisRect()->axis(QCPAxis::atRight, 1)->selectedParts().testFlag(QCPAxis::spAxis) ||
            this->axisRect()->axis(QCPAxis::atRight, 1)->selectedParts().testFlag(QCPAxis::spTickLabels)) {
            this->axisRect()->axis(QCPAxis::atRight, 1)->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    //    } else if (this->axisRect()->axis(QCPAxis::atRight, 2)->selectedParts().testFlag(QCPAxis::spAxis) ||
    //               this->axisRect()->axis(QCPAxis::atRight, 2)->selectedParts().testFlag(QCPAxis::spTickLabels)) {
    //        this->axisRect()->axis(QCPAxis::atRight, 2)->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    //    } else if (this->axisRect()->axis(QCPAxis::atRight, 3)->selectedParts().testFlag(QCPAxis::spAxis) ||
    //               this->axisRect()->axis(QCPAxis::atRight, 3)->selectedParts().testFlag(QCPAxis::spTickLabels)) {
    //        this->axisRect()->axis(QCPAxis::atRight, 3)->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
        }
    }

    // synchronize selection of graphs with selection of corresponding legend items:
    this->sync_selection();
}

void CustomPlotBase::mousePressEvent(QMouseEvent *event) {
    switch (event->button()) {
        case Qt::MiddleButton:
            this->yAxis->rescale();
            this->yAxis2->rescale();
            this->xAxis->rescale();
            if (this->axisRect()->axis(QCPAxis::atRight, 1)) {
                this->axisRect()->axis(QCPAxis::atRight, 1)->rescale();
            }
            this->replot();
            return;
            break;
        default:

            break;
    }

    QCustomPlot::mousePressEvent(event);
}

void CustomPlotBase::rescale(int index) {
    if (this->_autorescale) {
        //this->rescaleAxes();
        this->xAxis->rescale();
        if ((index % 2) == 0) {
            this->yAxis->rescale();
        } else {
            if (this->axisRect()->axis(QCPAxis::atRight, 1)) {
                this->axisRect()->axis(QCPAxis::atRight, 1)->rescale();
            }
        }
    }
    this->replot(QCustomPlot::rpQueuedReplot); //this->replot(); //
}

void CustomPlotBase::autoscale_all() {
    if (this->_autorescale)
        this->_autorescale = 0;
    else
        this->_autorescale = 1;
}

void CustomPlotBase::export_csv() {
    QString dir = "График " + QDateTime::currentDateTime().toString(DT_FORMAT_WITHOUT_MS) + ".csv"; //cfg.TEST_RESULT_DIR +
    QString filename = QFileDialog::getSaveFileName(this,
            tr("%1 - Сохранить").arg(QApplication::applicationDisplayName()),
            dir,
            tr("(*.csv)"),
            nullptr,
            QFileDialog::DontUseNativeDialog);

    if (filename != "") {
        QScopedPointer<QFile> file(new QFile(filename));
        if (file.data()->open(QFile::WriteOnly | QIODevice::Append | QFile::Text)) {
            QTextStream stream(file.data());
            this->export_data(stream);
            file->close();
        }
    }
};

void CustomPlotBase::export_png() {
    QString dir = "График " + QDateTime::currentDateTime().toString(DT_FORMAT_WITHOUT_MS) + ".png"; //cfg.TEST_RESULT_DIR +
    QString filename = QFileDialog::getSaveFileName(this,
            tr("%1 - Сохранить").arg(QApplication::applicationDisplayName()),
            dir,
            tr("(*.png)"),
            nullptr,
            QFileDialog::DontUseNativeDialog);

    if (filename != "") {
        this->savePng(filename);
    }
};
