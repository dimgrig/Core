#ifndef CONNECTIONBUTTON_H
#define CONNECTIONBUTTON_H

#include <QStateMachine>
#include <QState>
#include <QEventTransition>
#include <QPushButton>
#include <QIcon>

#include "core-widgets-lib-global.h"

class CORE_WIDGETS_LIB_EXPORT ConnectionButton : public QPushButton {
    Q_OBJECT
public:
    explicit ConnectionButton(QWidget *parent = nullptr);
    ~ConnectionButton();
signals:
    // Сигналы для внешнего класса управления сокетами
    void connectRequested();
    void disconnectRequested();

    void socketConnecting();
    void socketConnected();
    void socketDisconnected();
public slots:
    // Слоты для получения состояния от внешнего класса сокетов
    void onConnecting();
    void onConnected();
    void onDisconnected();
private slots:
    void handleClick();
private:
    void setupStateMachine();

private:
    QStateMachine *machine;
    QState *disconnectedState;
    QState *connectingState;
    QState *connectedState;
};

#endif // CONNECTIONBUTTON_H
