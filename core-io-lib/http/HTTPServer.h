#ifndef MODBUSSERVER_H
#define MODBUSSERVER_H

#include <QWidget>

#include "HttpRequestMapper.h"

#include "QtWebApp/httpserver/httplistener.h"

#include "mixins.h"

QT_BEGIN_NAMESPACE

//namespace Ui { class fhttp_server; }

QT_END_NAMESPACE

class SAKS_LIB_EXPORT HTTPServer : public QWidget, public NoCopyNoMoveBase {
Q_OBJECT
public:
    explicit HTTPServer(QWidget *parent = nullptr);
    ~HTTPServer();

    void set_finished(bool finished) {_finished = finished;};
    bool get_finished() const {return _finished;};
signals:
    void read_ready();
public slots:
    HttpRequestMapper* get_request_mapper() const {return _request_mapper;};

    //void process(const TestSequence *tests);
private slots:

private:
    //Ui::fhttp_server *ui;

    HttpRequestMapper* _request_mapper;
    stefanfrings::HttpListener* _http_listener;

    bool _finished = false;
};

#endif // MODBUSSERVER_H
