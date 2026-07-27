#include "AboutPanel.h"
#include "ui_about.h"

#include "IconManager.h"

AboutPanel::AboutPanel(QWidget *parent,
                       QPixmap organization_pixmap,
                       QPixmap application_pixmap,
                       QString organizationName,
                       QString applicationName,
                       QString version)
    : QWidget(parent)
    , ui(new Ui::fabout)
    , _organization_pixmap(organization_pixmap)
    , _application_pixmap(application_pixmap)
    , _organizationName(organizationName)
    , _applicationName(applicationName)
    , _version(version) {
    ui->setupUi(this);

    this->setWindowTitle("О программе");

    this->setWindowFlags(Qt::Window);

    this->ui->l_organization_pixmap->setPixmap(_organization_pixmap);
    this->ui->l_application_pixmap->setPixmap(_application_pixmap);
}

AboutPanel::~AboutPanel()
{
    delete ui;
}

void AboutPanel::read_settings() {
    this->ui->pte_ORGANIZATION_NAME->setPlainText(_organizationName);
    this->ui->pte_APPLICATION_NAME->setPlainText(_applicationName);
    this->ui->pte_VERSION->setPlainText(_version);
}

void AboutPanel::showEvent(QShowEvent *event) {
    this->read_settings();

    QWidget::showEvent(event);
}
