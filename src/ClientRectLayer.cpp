#include "ClientRectLayer.h"
#include <QLabel>

ClientRectLayer::ClientRectLayer(QWidget *parent) : QWidget(parent) {
    // Remove debug styling for cleaner appearance
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
}