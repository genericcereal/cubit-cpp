#include "ActionsPanel.h"
#include "Canvas.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVariant>
#include <QFrame>
#include <QMouseEvent>
#include <QList>
#include <QStringList>

class Action : public QFrame {
public:
    Action(ActionsPanel *panel, const QString &modeName, QWidget *parent = nullptr) 
        : QFrame(parent), selected(false), parentPanel(panel), mode(modeName) {
        setFixedSize(45, 45);
        setObjectName("action");
        updateStyle();
    }

    void setSelected(bool sel) {
        selected = sel;
        updateStyle();
    }

    bool isSelected() const { return selected; }
    QString getMode() const { return mode; }

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    bool selected;
    ActionsPanel *parentPanel;
    QString mode;
    
    void updateStyle() {
        if (selected) {
            setStyleSheet("#action { background-color: #4169E1; border: 1px solid #888; }");
        } else {
            setStyleSheet("#action { background-color: #666; border: 1px solid #888; }"
                          "#action:hover { background-color: #888; }");
        }
    }
};

ActionsPanel::ActionsPanel(QWidget *parent) : QFrame(parent), canvasRef(nullptr) {
    setObjectName("actionsPanel");
    setFixedSize(400, 50);
    setProperty("panelStyle", QVariant("true"));
    setAutoFillBackground(true);            // ensure background paints
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(5,2,5,2);
    layout->setSpacing(5);

    // Define modes for each action
    QStringList modes = {"Select", "Frame", "Text", "Variable"};
    
    // Add 4 Action boxes with their modes
    for (int i = 0; i < modes.size(); ++i) {
        auto *action = new Action(this, modes[i], this);
        actions.append(action);
        layout->addWidget(action);
    }
    
    // Select the first action by default
    if (!actions.isEmpty()) {
        actions[0]->setSelected(true);
    }

    layout->addStretch();
}

void ActionsPanel::onActionClicked(Action *clickedAction) {
    // Deselect all actions
    for (Action *action : actions) {
        action->setSelected(false);
    }
    // Select the clicked action
    clickedAction->setSelected(true);
    
    // Update canvas mode if canvas is set
    if (canvasRef && canvasRef->getMode() != clickedAction->getMode()) {
        canvasRef->setMode(clickedAction->getMode());
    }
}

void ActionsPanel::setCanvas(Canvas *canvas) {
    canvasRef = canvas;
}

void ActionsPanel::setModeSelection(const QString &modeName) {
    // Find the action with the matching mode and select it
    for (Action *action : actions) {
        if (action->getMode() == modeName) {
            onActionClicked(action);
            break;
        }
    }
}

void Action::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && parentPanel) {
        parentPanel->onActionClicked(this);
        event->accept();  // Accept the event to stop propagation
        return;
    }
    QFrame::mousePressEvent(event);
}