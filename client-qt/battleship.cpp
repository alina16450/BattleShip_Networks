#include "battleship.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>

battleship::battleship(QWidget* parent)
    : QMainWindow(parent)
{
    setupUi();
}

void battleship::setupUi()
{
    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout* mainLayout   = new QVBoxLayout;
    QHBoxLayout* boardLayout  = new QHBoxLayout;
    QHBoxLayout* bottomLayout = new QHBoxLayout;

    myBoard    = new QTableWidget(10, 10, this);
    enemyBoard = new QTableWidget(10, 10, this);

    chatmanager = new class chatmanager(this);

    setupBoards();

    boardLayout->addWidget(myBoard);
    boardLayout->addWidget(enemyBoard);
    mainLayout->addLayout(boardLayout);

    attackButton = new QPushButton("Attack", this);
    bottomLayout->addWidget(attackButton);

    moves = new QLabel("Moves left:", this);
    movesLeftLabel = new QLabel("", this);
    bottomLayout->addWidget(moves);
    bottomLayout->addWidget(movesLeftLabel);

    connect(attackButton, &QPushButton::clicked,
            this, &battleship::onAttackButtonClicked);

    chatBox = new QTextEdit(this);
    chatBox->setReadOnly(true);
    chatBox->setMinimumHeight(160);

    chatInput = new QLineEdit(this);
    sendButton = new QPushButton("Send", this);

    QHBoxLayout* chatInputLayout = new QHBoxLayout();
    chatInputLayout->addWidget(chatInput);
    chatInputLayout->addWidget(sendButton);

    mainLayout->addLayout(bottomLayout);
    mainLayout->addWidget(chatBox);
    mainLayout->addLayout(chatInputLayout);

    connect(sendButton, &QPushButton::clicked,
            this, &battleship::sendChatMessage);

    connect(chatInput, &QLineEdit::returnPressed,
            this, &battleship::sendChatMessage);

    connect(chatmanager, &chatmanager::displayMessage,
            this, [&](const QString& msg) {
                chatBox->append(msg);
            });

    net = new networkmanager(this);

    connect(net, &networkmanager::udpMessageReceived,
            this, [&](const QString& msg){

                if (msg.startsWith("CHAT ")) {
                    QString text = msg.mid(5);  // strip "CHAT "
                    chatBox->append(text);
                }
                else {
                    chatBox->append("[UDP] " + msg);
                }
            });

    connect(net, &networkmanager::tcpMessageReceived,
            this, &battleship::processTcpMessage);

    net->sendUdpMessage("CHAT JOIN");

    central->setLayout(mainLayout);
}

void battleship::setupBoards()
{
    QTableWidget* tables[2] = { myBoard, enemyBoard };

    for (int b = 0; b < 2; ++b)
    {
        QTableWidget* table = tables[b];

        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        table->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->setSelectionMode(QAbstractItemView::SingleSelection);
        table->setSelectionBehavior(QAbstractItemView::SelectItems);

        for (int i = 0; i < 10; ++i)
        {
            for (int j = 0; j < 10; ++j)
            {
                table->setItem(i, j, new QTableWidgetItem(" "));
            }
        }
    }
}

void battleship::sendChatMessage()
{
    QString msg = chatInput->text().trimmed();
    if (msg.isEmpty()) return;

    if (msg.startsWith("CHAT JOIN"))
        return;

    QString packet = "CHAT " + msg;
    net->sendUdpMessage(packet);

    chatInput->clear();
}

void battleship::processTcpMessage(const QString& msg)
{
    QStringList parts = msg.split(' ', Qt::SkipEmptyParts);
    if (parts.isEmpty())
        return;

    const QString& cmd = parts[0];

    // ROLE P1 / ROLE P2
    if (cmd == "ROLE" && parts.size() >= 2)
    {
        if (parts[1] == "P1")
            myPlayerId = 1;
        else if (parts[1] == "P2")
            myPlayerId = 2;

        chatBox->append("You are " + parts[1]);
        return;
    }

    if (cmd == "BOARDROW" && parts.size() == 12)
    {
        int row = parts[1].toInt();
        for (int col = 0; col < 10; ++col)
        {
            int val = parts[col + 2].toInt();
            if (val == 1)
                myBoard->item(row, col)->setBackground(Qt::gray);
        }
        return;
    }

    if (cmd == "BOARDEND")
    {
        chatBox->append("Board received from server.");
        return;
    }

    if (cmd == "RESULT" && parts.size() >= 4)
    {
        int x = parts[1].toInt();
        int y = parts[2].toInt();
        QString outcome = parts[3];

        if (!enemyBoard->item(x, y))
            enemyBoard->setItem(x, y, new QTableWidgetItem(" "));

        if (outcome == "HIT") {
            enemyBoard->item(x, y)->setText("*");
            enemyBoard->item(x, y)->setBackground(Qt::red);
        } else if (outcome == "MISS") {
            enemyBoard->item(x, y)->setText("o");
            enemyBoard->item(x, y)->setBackground(Qt::blue);
        } else if (outcome == "ALREADY") {
            QMessageBox::information(this, "Info", "You already attacked that cell.");
        }
        return;
    }

    if (cmd == "INCOMING" && parts.size() >= 4)
    {
        int x = parts[1].toInt();
        int y = parts[2].toInt();
        QString outcome = parts[3];

        if (!myBoard->item(x, y))
            myBoard->setItem(x, y, new QTableWidgetItem(" "));

        if (outcome == "HIT") {
            myBoard->item(x, y)->setBackground(Qt::red);
        } else if (outcome == "MISS") {
            myBoard->item(x, y)->setBackground(Qt::blue);
        }
        return;
    }

    if (cmd == "TURN" && parts.size() >= 2)
    {
        QString who = parts[1];
        bool myTurn = (who == "P1" && myPlayerId == 1)
                      || (who == "P2" && myPlayerId == 2);

        attackButton->setEnabled(myTurn);
        chatBox->append(QString("Turn: %1").arg(who));
        return;
    }

    if (cmd == "WIN")
    {
        QMessageBox::information(this, "Game Over", "You win!");
        attackButton->setEnabled(false);
        return;
    }

    if (cmd == "LOSE")
    {
        QMessageBox::information(this, "Game Over", "You lose!");
        attackButton->setEnabled(false);
        return;
    }

    chatBox->append("[TCP] " + msg);
}

void battleship::onAttackButtonClicked()
{
    int i = enemyBoard->currentRow();
    int j = enemyBoard->currentColumn();

    if (i < 0 || j < 0)
    {
        QMessageBox::warning(this, "Error", "Select a cell to attack!");
        return;
    }

    QString attackMsg = QString("ATTACK %1 %2").arg(i).arg(j);
    net->sendTcpMessage(attackMsg);

    attackButton->setEnabled(false);
}

void battleship::updateCount()
{
    movesLeftLabel->setText("");
}
