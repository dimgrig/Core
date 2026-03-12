#include "AboutPanel.h"
#include "ui_about.h"

AboutPanel::AboutPanel(QWidget *parent,
                       QPixmap pixmap,
                       QString organizationName,
                       QString applicationName,
                       QString version)
    : QWidget(parent)
    , ui(new Ui::fabout)
    , _pixmap(pixmap)
    , _organizationName(organizationName)
    , _applicationName(applicationName)
    , _version(version) {
    ui->setupUi(this);

    this->setWindowTitle("О программе");

    this->setWindowFlags(Qt::Window);

    this->ui->l_application_pixmap->setPixmap(_pixmap);
}

AboutPanel::~AboutPanel()
{
    delete ui;
}

void AboutPanel::read_settings() {
    this->ui->le_ORGANIZATION_NAME->setText(_organizationName);
    this->ui->le_APPLICATION_NAME->setText(_applicationName);
    this->ui->le_VERSION->setText(_version);
}

void AboutPanel::showEvent(QShowEvent *event) {
    this->read_settings();

    QWidget::showEvent(event);
}
