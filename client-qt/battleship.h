#ifndef BATTLESHIP_H
#define BATTLESHIP_H

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>

#include "networkmanager.h"
#include "chatmanager.h"

class battleship : public QMainWindow
{
    Q_OBJECT

public:
    explicit battleship(QWidget *parent = nullptr);

private:
    // UI
    QTableWidget* myBoard;
    QTableWidget* enemyBoard;

    QPushButton* attackButton;
    QTextEdit*   chatBox;
    QLineEdit*   chatInput;
    QPushButton* sendButton;

    QLabel* moves;
    QLabel* movesLeftLabel;

    networkmanager* net;
    chatmanager*    chatmanager;

    int myPlayerId = 0;
    int totalMoves = 30;

    void setupUi();
    void setupBoards();
    void processTcpMessage(const QString& msg);
    void sendChatMessage();

private slots:
    void onAttackButtonClicked();
    void updateCount();
};

#endif // BATTLESHIP_H
