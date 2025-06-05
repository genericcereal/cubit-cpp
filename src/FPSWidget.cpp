#include "FPSWidget.h"

#include <QLabel>
#include <QTimer>
#include <QElapsedTimer>
#include <QVBoxLayout>
#include <QDateTime>

FPSWidget::FPSWidget(QWidget *parent)
    : QWidget(parent)
    , currentFPS(0)
    , targetFPS(60)
    , frameCount(0)
    , lastTime(0)
    , running(false)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    
    fpsLabel = new QLabel("FPS: 0", this);
    fpsLabel->setAlignment(Qt::AlignCenter);
    fpsLabel->setStyleSheet("QLabel { font-family: monospace; font-size: 14px; color: #00ff00; background-color: #000000; padding: 5px; border: 1px solid #333333; }");
    
    layout->addWidget(fpsLabel);
    
    updateTimer = new QTimer(this);
    updateTimer->setInterval(1000 / targetFPS);
    connect(updateTimer, &QTimer::timeout, this, &FPSWidget::updateFPS);
}

FPSWidget::~FPSWidget()
{
    stop();
}

void FPSWidget::setTargetFPS(int fps)
{
    if (fps > 0 && fps <= 1000) {
        targetFPS = fps;
        updateTimer->setInterval(1000 / targetFPS);
    }
}

void FPSWidget::start()
{
    if (!running) {
        running = true;
        frameCount = 0;
        lastTime = QDateTime::currentMSecsSinceEpoch();
        updateTimer->start();
    }
}

void FPSWidget::stop()
{
    if (running) {
        running = false;
        updateTimer->stop();
        currentFPS = 0;
        fpsLabel->setText("FPS: 0");
    }
}

void FPSWidget::updateFPS()
{
    frameCount++;
    calculateFPS();
}

void FPSWidget::calculateFPS()
{
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    qint64 elapsedTime = currentTime - lastTime;
    
    if (elapsedTime >= 1000) {
        currentFPS = static_cast<int>((frameCount * 1000.0) / elapsedTime);
        
        fpsLabel->setText(QString("FPS: %1").arg(currentFPS));
        
        emit fpsUpdated(currentFPS);
        
        frameCount = 0;
        lastTime = currentTime;
    }
}