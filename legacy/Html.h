#pragma once
#include "Element.h"
#include <QString>

QT_BEGIN_NAMESPACE
class QWebEngineView;
QT_END_NAMESPACE

class Html : public Element {
    Q_OBJECT
public:
    explicit Html(int id, QWidget *parent = nullptr);
    
    // Override Element virtual methods
    int getId() const override { return elementId; }
    QString getName() const override { return htmlName; }
    
    void setHtml(const QString &html);
    QString getHtml() const { return content; }
    
    // Override visual update
    void updateParentVisualState() override;

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    QString htmlName;
    QString content;
    QWebEngineView *webView;
};