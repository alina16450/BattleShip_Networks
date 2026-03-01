#include "chatmanager.h"

chatmanager::chatmanager(QObject* parent) : QObject(parent) {}

void chatmanager::sendMessage(const QString& msg){
    if (msg.trimmed().isEmpty())
        return;
    QString formatted = "You: " + msg;
    emit displayMessage(formatted);
}

void chatmanager::receiveMessage(const QString& msg){
    QString formatted = "Opponent: " + msg;
    emit displayMessage(formatted);
}
