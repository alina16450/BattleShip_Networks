#ifndef CHATMANAGER_H
#define CHATMANAGER_H

#include <QObject>
#include <QString>

class chatmanager : public QObject
{
    Q_OBJECT
public:
    explicit chatmanager(QObject* parent = nullptr);

    void sendMessage(const QString& msg);

    void receiveMessage(const QString& msg);

signals:
    void displayMessage(const QString& msg);
};

#endif // CHATMANAGER_H
