#include "DetailPanel.h"
#include "ElementList.h"
#include "Properties.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QSplitter>
#include <QWidget>
#include <QVariant>

DetailPanel::DetailPanel(QWidget *parent) : QFrame(parent) {
    // Apply global panel style
    setProperty("panelStyle", true);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0); // 🔥 remove outer margin
    mainLayout->setSpacing(0);                 // 🔥 remove spacing between widgets
    QSplitter *splitter = new QSplitter(Qt::Vertical, this);

    // Element List section
    elementList = new ElementList();
    QScrollArea *scrollA = new QScrollArea();
    scrollA->setWidget(elementList);
    scrollA->setWidgetResizable(true);

    // Properties section
    Properties *properties = new Properties();
    QScrollArea *scrollB = new QScrollArea();
    scrollB->setWidget(properties);
    scrollB->setWidgetResizable(true);

    // Add to vertical splitter
    splitter->addWidget(scrollA);
    splitter->addWidget(scrollB);
    splitter->setSizes({200, 200}); // Optional: initial height of sections

    mainLayout->addWidget(splitter);
}