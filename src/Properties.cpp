#include "Properties.h"
#include <QVBoxLayout>
#include <QLabel>

Properties::Properties(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Bottom scrollable content"));
    for (int i = 0; i < 20; ++i) {
        layout->addWidget(new QLabel("Item B " + QString::number(i + 1)));
    }
    layout->addStretch();
}