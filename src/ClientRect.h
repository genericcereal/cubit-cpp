#pragma once
#include <QFrame>

class ClientRect : public QFrame {
    Q_OBJECT
public:
    explicit ClientRect(QWidget *parent = nullptr);
};