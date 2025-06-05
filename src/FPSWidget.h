#pragma once

#include <QWidget>

class QLabel;
class QTimer;

class FPSWidget : public QWidget {
    Q_OBJECT
public:
    explicit FPSWidget(QWidget *parent = nullptr);
    ~FPSWidget();
    
    int getCurrentFPS() const { return currentFPS; }
    void setTargetFPS(int fps);
    
    void start();
    void stop();
    bool isRunning() const { return running; }
    
signals:
    void fpsUpdated(int fps);
    
private slots:
    void updateFPS();
    
private:
    void calculateFPS();
    
    QLabel *fpsLabel;
    QTimer *updateTimer;
    int currentFPS;
    int targetFPS;
    int frameCount;
    qint64 lastTime;
    bool running;
};