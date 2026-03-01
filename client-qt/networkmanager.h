#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QUdpSocket>
#include <QTcpSocket>
#include <QHostAddress>

class networkmanager : public QObject
{
    Q_OBJECT

public:
    QTcpSocket* tcpSocket;

    explicit networkmanager(QObject* parent = nullptr);

    void sendUdpMessage(const QString& message);
    void sendTcpMessage(const QString& msg);

private slots:
    void onUdpReadyRead();
    void onTcpReadyRead();
    void onTcpConnected();
    void onTcpDisconnected();

signals:
    void udpMessageReceived(const QString& message);
    void tcpMessageReceived(const QString& message);

private:
    QUdpSocket* udpSocket;
    QHostAddress serverAddress;
    quint16 serverPort;
};

#endif // NETWORKMANAGER_H
