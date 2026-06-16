#include "CustomPlotSettings.h"

//CustomPlotSettings::CustomPlotSettings(QWidget *parent) :
//    QWidget(parent),
//    ui(new Ui::fcp_settings)
//{
//    ui->setupUi(this);

//    this->ui->cb_channel->addItem(QString("Не выбран"), 0);
//    for (int i = 1; i <= cfg.CHANNELS; ++i) {
//        this->ui->cb_channel->addItem(QString("Канал ") + QString::number(i), i);
//    }
//    this->ui->sb_data_size->setMaximum(cfg.PLOT_SIZE);
//    this->ui->sb_data_size->setValue(cfg.PLOT_SIZE);

//    connect(this->ui->sb_data_size, QOverload<int>::of(&QSpinBox::valueChanged), this, &CustomPlotSettings::data_size_changed);
//    connect(this->ui->cb_channel, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CustomPlotSettings::channel_changed);
//    connect(this->ui->cb_autorescale, &QCheckBox::stateChanged, this, &CustomPlotSettings::autorescale_changed);
//}

//CustomPlotSettings::~CustomPlotSettings() {

//}

