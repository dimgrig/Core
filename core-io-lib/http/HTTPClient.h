#ifndef MODBUSCLIENT_H
#define MODBUSCLIENT_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>

#include "saks-lib-global.h"

class SAKS_LIB_EXPORT HTTPClient : public QWidget {
    Q_OBJECT
public:
    HTTPClient(QWidget *parent = nullptr);
    ~HTTPClient();

signals:
    void read_data(QByteArray data);
public slots:
    void connect_to_host(const QUrl url);
    void request(const QUrl url = QUrl(), const QJsonObject *requestObject = nullptr);
private slots:
    void on_read_ready();
    void http_finished();
    void authentication_required(QNetworkReply *, QAuthenticator *authenticator);
#ifndef QT_NO_SSL
    void ssl_errors(QNetworkReply *, const QList<QSslError> &errors);
#endif
private:
    QUrl _url;
    QNetworkAccessManager _qnam;
    //QNetworkReply *_reply;
    bool _httpRequestAborted = false;

    bool _request = false;
};

#endif // MODBUSCLIENT_H

