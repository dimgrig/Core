#include "HTTPClient.h"
#include "qjsondocument.h"
#include "qjsonobject.h"
//#include "ui_authenticationdialog.h"

#include <QAuthenticator>
#include <QUrl>
#include <QUrlQuery>

#include "Logger.h"

HTTPClient::HTTPClient(QWidget *parent)
    : QWidget(parent)
{
    // QNetworkConfiguration configuration = _qnam.configuration();
    // _session = QNetworkSession(configuration, this);

    connect(&_qnam, &QNetworkAccessManager::authenticationRequired,
            this, &HTTPClient::authentication_required);
#ifndef QT_NO_SSL
    connect(&_qnam, &QNetworkAccessManager::sslErrors,
            this, &HTTPClient::ssl_errors);
#endif

    // connect(&_session, &QNetworkSession::opened, this, [=](){
    //     qCDebug(logInfo()) << "HTTP: connected to" << _url;
    //     emit host_connected();
    // });
    // connect(&_session, &QNetworkSession::closed, this, [=](){
    //     qCDebug(logInfo()) << "HTTP: disconnected from" << _url;
    //     emit host_disconnected();
    // });
}

HTTPClient::~HTTPClient() {
    _qnam.disconnect();
}

void HTTPClient::set_async(bool async) {
    _async = async;
    // if (_reply != nullptr) {
    //     if (_async) {
    //         connect(_reply, &QNetworkReply::finished, this, &HTTPClient::http_finished);
    //         connect(_reply, &QIODevice::readyRead, this, &HTTPClient::readDataSlot);
    //     } else {
    //         disconnect(_reply, &QNetworkReply::finished, this, &HTTPClient::http_finished);
    //         disconnect(_reply, &QIODevice::readyRead, this, &HTTPClient::readDataSlot);
    //     }
    // }
}

void HTTPClient::connectToHost(const QString url) {
    QUrl _url = QUrl(url);
    this->connectToHost(_url);
}

void HTTPClient::connectToHost(const QUrl url) {
    _qnam.connectToHost(url.host(), url.port());
    //QNetworkConfiguration configuration = _qnam.configuration();
    //_session = QNetworkSession(configuration, this);
}

void HTTPClient::disconnectFromHost() {
    if (this->isConnected()) {
        _qnam.disconnect();
    }
}

std::string HTTPClient::readData() {

    // //while(_session->()){
    //     rxBuff += reply->readAll();

    //     if (cfg.DEBUG_MSG_TEST_INTERFACES) {
    //         qCDebug(logDebug()) << QT_HEX << "HTTP: rx - " << qPrintable(rxBuff.toHex(' ')) << "SIZE" << rxBuff.size();
    //     }
    // //}

    // this->http_finished();

    QJsonDocument responseDocument = QJsonDocument::fromJson(_rxBuff);
    QJsonObject responseObject = responseDocument.object();
    if (!responseObject.empty()) {
        if (responseObject.keys().size() == 1) {
            if (responseObject.value(responseObject.keys().front()).isString()) {
                return responseObject.value(responseObject.keys().front()).toString().toStdString();
            } else if (responseObject.value(responseObject.keys().front()).isDouble()) {
                return QString::number(responseObject.value(responseObject.keys().front()).toDouble()).toStdString();
            }
        }
    }

    return _rxBuff.toStdString();
}

void HTTPClient::request(const QUrl url, const QJsonObject *requestObject) {
    //if (_session.isOpen()) {
        if (!url.isEmpty()) {
            _url = url;
        }

        _rxBuff.clear();

        _httpRequestAborted = false;

        QNetworkReply *reply;
        QNetworkRequest request(_url);

        request.setRawHeader("Connection", "close");

        if (requestObject) {
            request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
            QJsonDocument requestDoc(*requestObject);
            reply = _qnam.post(request, requestDoc.toJson());

            // QUrlQuery params;
            // params.addQueryItem("freq", "1.0");
            // params.addQueryItem("duty", "10");
            // request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
            // reply = _qnam.post(request, params.query(QUrl::FullyEncoded).toUtf8());
        } else {
            reply = _qnam.get(request);
        }

        // if (_async) {
            connect(reply, &QNetworkReply::finished, this, &HTTPClient::http_finished);
            connect(reply, &QIODevice::readyRead, this, &HTTPClient::readDataSlot);
        // }
    //}
}

void HTTPClient::readDataSlot() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    _rxBuff = reply->readAll();

    //qCDebug(logDebug()) << "HTTP readDataSlot" <<_rxBuff;

    emit read_data(_rxBuff);
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
