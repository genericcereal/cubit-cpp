#pragma once
#include <QWidget>
#include <QList>

class QVBoxLayout;
class QLabel;

class ElementList : public QWidget {
    Q_OBJECT
public:
    explicit ElementList(QWidget *parent = nullptr);
    
public slots:
    void addElement(const QString &type, const QString &name);
    void clear();
    
private:
    QVBoxLayout *layout;
    QLabel *headerLabel;
    QList<QLabel*> frameLabels;
    int frameCount;
};