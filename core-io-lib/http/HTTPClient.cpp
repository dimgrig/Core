#include "HTTPClient.h"
#include "qjsondocument.h"
#include "qjsonobject.h"
//#include "ui_authenticationdialog.h"

#include <QAuthenticator>
#include <QUrl>

#include "Logger.h"

HTTPClient::HTTPClient(QWidget *parent)
    : QWidget(parent)
{
    connect(&_qnam, &QNetworkAccessManager::authenticationRequired,
            this, &HTTPClient::authentication_required);
#ifndef QT_NO_SSL
    connect(&_qnam, &QNetworkAccessManager::sslErrors,
            this, &HTTPClient::ssl_errors);
#endif
}

HTTPClient::~HTTPClient() {
    _qnam.disconnect();
//    if (modbusDevice)
//        modbusDevice->disconnectDevice();
//    delete modbusDevice;
}

void HTTPClient::connect_to_host(const QUrl url) {
    _qnam.connectToHost(url.host(), url.port());
}

void HTTPClient::request(const QUrl url, const QJsonObject *requestObject) {
    if (!url.isEmpty()) {
        _url = url;        
    }
    _httpRequestAborted = false;

    QNetworkReply *reply;
    QNetworkRequest request(_url);
    if (requestObject) {
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        QJsonDocument requestDoc(*requestObject);
        reply = _qnam.post(request, requestDoc.toJson());
    } else {
        reply = _qnam.get(request);
    }
    connect(reply, &QNetworkReply::finished, this, &HTTPClient::http_finished);
    connect(reply, &QIODevice::readyRead, this, &HTTPClient::on_read_ready);
}

void HTTPClient::on_read_ready() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    emit read_data(reply->readAll());
}

void HTTPClient::http_finished() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (_httpRequestAborted) {
        reply->deleteLater();
        reply = nullptr;
        return;
    }

    if (reply->error()) {
        qCDebug(logDebug) << "HHTP: Error" << reply->error();
        reply->deleteLater();
        reply = nullptr;
        return;
    }

    const QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

    reply->deleteLater();
    reply = nullptr;

    if (!redirectionTarget.isNull()) {
        const QUrl redirectedUrl = _url.resolved(redirectionTarget.toUrl());
        qCDebug(logDebug) << "HHTP: redirect" << redirectedUrl;
        this->request(redirectedUrl);
        return;
    }
}

void HTTPClient::authentication_required(QNetworkReply *, QAuthenticator *authenticator) {
    Q_UNUSED(authenticator);
//    QDialog authenticationDialog;
//    Ui::Dialog ui;
//    ui.setupUi(&authenticationDialog);
//    authenticationDialog.adjustSize();
//    ui.siteDescription->setText(tr("%1 at %2").arg(authenticator->realm(), _url.host()));

//    // Did the URL have information? Fill the UI
//    // This is only relevant if the URL-supplied credentials were wrong
//    ui.userEdit->setText(_url.userName());
//    ui.passwordEdit->setText(_url.password());

//    if (authenticationDialog.exec() == QDialog::Accepted) {
//        authenticator->setUser(ui.userEdit->text());
//        authenticator->setPassword(ui.passwordEdit->text());
//    }
}

#ifndef QT_NO_SSL
void HTTPClient::ssl_errors(QNetworkReply *, const QList<QSslError> &errors) {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    QString errorString;
    for (const QSslError &error : errors) {
        if (!errorString.isEmpty())
            errorString += '\n';
        errorString += error.errorString();
    }

    reply->ignoreSslErrors();
//    if (AQP::question(this, tr("SSL Errors"),
//                 tr("One or more SSL errors has occurred:\n%1").arg(errorString),
//                 "")) {
//        _reply->ignoreSslErrors();
//    }

//    if (QMessageBox::warning(this, tr("SSL Errors"),
//                             tr("One or more SSL errors has occurred:\n%1").arg(errorString),
//                             QMessageBox::Ignore | QMessageBox::Abort) == QMessageBox::Ignore) {
//        _reply->ignoreSslErrors();
//    }
}
#endif
