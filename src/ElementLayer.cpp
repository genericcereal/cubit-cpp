#include "ElementLayer.h"
#include <QLabel>

ElementLayer::ElementLayer(QWidget *parent) : QWidget(parent) {
    // Remove debug styling for cleaner appearance
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
}