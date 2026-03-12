#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkSession>
#include <QUrl>

#include "core-io-lib-global.h"

class CORE_IO_LIB_EXPORT HTTPClient : public QWidget {
    Q_OBJECT
public:
    HTTPClient(QWidget *parent = nullptr);
    ~HTTPClient();

    void set_async(bool async);
    bool isConnected() {return true;}; //{return this->_session.isOpen();};
    ////bool isReadable() {return this->_session->isReadable();};
    int bytesAvailable() {return _rxBuff.size();}; //{return this->_session.bytesReceived();};
    ////bool waitForReadyRead(int msecs = 1000) {return this->_session->waitForReadyRead(msecs);};
    ////bool reset() const {return this->_session->reset();};
signals:
    //void host_error(QString error);
    void host_connected();
    void host_disconnected();
    void read_data(QByteArray data);
public slots:
    void connectToHost(const QString url);
    void connectToHost(const QUrl url);
    void disconnectFromHost();
    std::string readData();
    void request(const QUrl url = QUrl(), const QJsonObject *requestObject = nullptr);
private slots:
    void readDataSlot();
    void http_finished();
    void authentication_required(QNetworkReply *, QAuthenticator *authenticator);
#ifndef QT_NO_SSL
    void ssl_errors(QNetworkReply *, const QList<QSslError> &errors);
#endif
private:
    QUrl _url;
    QNetworkAccessManager _qnam;
    //QNetworkSession _session;
    QNetworkReply *_reply;

    bool _httpRequestAborted = false;
    bool _request = false;
    bool _async = false;

    QByteArray _rxBuff;
};

#endif // HTTPCLIENT_H

