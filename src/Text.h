#pragma once
#include "Element.h"
#include <QString>

class Text : public Element {
    Q_OBJECT
public:
    explicit Text(int id, QWidget *parent = nullptr);
    
    // Override Element virtual methods
    int getId() const override { return textId; }
    QString getName() const override { return textName; }
    
    void setText(const QString &text);
    QString getText() const { return content; }
    
    // Editing functionality
    void startEditing();
    void endEditing(bool save = true);
    bool isEditing() const { return editing; }

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    int textId;
    QString textName;
    QString content;
    bool editing;
    class QTextEdit *editWidget;
};