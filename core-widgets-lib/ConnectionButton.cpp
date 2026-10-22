#include "ConnectionButton.h"

#include "IconManager.h"
#include "Logger.h"

ConnectionButton::ConnectionButton(QWidget *parent) : QPushButton(parent) {
    setIconSize(ICON_SIZE_24);

    setupStateMachine();

    // Обработка нажатия кнопки
    connect(this, &QPushButton::clicked, this, &ConnectionButton::handleClick);
}

ConnectionButton::~ConnectionButton() {

}

// Слоты для получения состояния от внешнего класса сокетов
void ConnectionButton::onConnecting() {
    emit socketConnecting();
}

void ConnectionButton::onConnected() {
    emit socketConnected();
}

void ConnectionButton::onDisconnected() {
    emit socketDisconnected();
}

void ConnectionButton::forceState(int state) {
    switch (state) {
    case 2:  // connected
        // QStateMachine не имеет прямого перехода disconnected->connected,
        // поэтому сначала эмитим socketConnecting (если в disconnected),
        // затем socketConnected.
        emit socketConnecting();
        emit socketConnected();
        break;
    case 1:  // connecting
        emit socketConnecting();
        break;
    case 0:  // disconnected
    default:
        emit socketDisconnected();
        break;
    }
}

void ConnectionButton::handleClick() {
    // В зависимости от текущего состояния отправляем соответствующий сигнал
    if (machine->configuration().contains(disconnectedState)) {
        emit connectRequested();
    } else if (machine->configuration().contains(connectedState)) {
        emit disconnectRequested();
    }
}

void ConnectionButton::setupStateMachine() {
    machine = new QStateMachine(this);

    // Создание состояний
    disconnectedState = new QState(machine);
    connectingState = new QState(machine);
    connectedState = new QState(machine);

    disconnectedState->addTransition(this, &ConnectionButton::socketConnecting, connectingState);
    connectingState->addTransition(this, &ConnectionButton::socketConnected, connectedState);
    connectingState->addTransition(this, &ConnectionButton::socketDisconnected, disconnectedState);
    connectedState->addTransition(this, &ConnectionButton::socketDisconnected, disconnectedState);

    // Настройка свойств состояний
    disconnectedState->assignProperty(this, "icon", ICON_DISCONNECTED);
    disconnectedState->assignProperty(this, "text", tr("Отключено"));
    disconnectedState->assignProperty(this, "enabled", true);

    connectingState->assignProperty(this, "icon", ICON_CONNECTING);
    connectingState->assignProperty(this, "text", tr("Подключение..."));
    connectingState->assignProperty(this, "enabled", false);

    connectedState->assignProperty(this, "icon", ICON_CONNECTED);
    connectedState->assignProperty(this, "text", tr("Подключено"));
    connectedState->assignProperty(this, "enabled", true);

    machine->setInitialState(disconnectedState);
    machine->start();

    //qCDebug(logDebug()) << this->iconSize();
}

