#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include <QString>
#include <QJsonObject>

class Project;
class Application;

class FileManager : public QObject
{
    Q_OBJECT

public:
    explicit FileManager(Application* app, QObject *parent = nullptr);
    
    // File operations
    bool saveAs();
    bool openFile();
    bool saveToFile(const QString& fileName, Project* project = nullptr);
    bool loadFromFile(const QString& fileName);
    
signals:
    void saveFileRequested();
    void openFileRequested();
    
private:
    Application* m_application;
};

#endif // FILEMANAGER_H