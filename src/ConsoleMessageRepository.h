#ifndef CONSOLEMESSAGEREPOSITORY_H
#define CONSOLEMESSAGEREPOSITORY_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QList>
#include <QVariantMap>

class ConsoleMessageRepository : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList messages READ messages NOTIFY messagesChanged)
    
public:
    enum MessageType {
        Input,
        Output,
        Error,
        Warning,
        Info
    };
    Q_ENUM(MessageType)
    
    explicit ConsoleMessageRepository(QObject *parent = nullptr);
    static ConsoleMessageRepository* instance();
    
    QVariantList messages() const;
    
    Q_INVOKABLE void addMessage(const QString &text, MessageType type = Output);
    Q_INVOKABLE void addInput(const QString &text) { addMessage(text, Input); }
    Q_INVOKABLE void addOutput(const QString &text) { addMessage(text, Output); }
    Q_INVOKABLE void addError(const QString &text) { addMessage(text, Error); }
    Q_INVOKABLE void addWarning(const QString &text) { addMessage(text, Warning); }
    Q_INVOKABLE void addInfo(const QString &text) { addMessage(text, Info); }
    
    Q_INVOKABLE void clearMessages();
    Q_INVOKABLE QString messageTypeToString(MessageType type) const;
    Q_INVOKABLE void processConsoleCommand(const QString& command);
    
signals:
    void messagesChanged();
    void messageAdded(const QString &text, MessageType type);
    void cubitAICommandReceived(const QString& prompt);
    
private:
    static ConsoleMessageRepository* s_instance;
    
    struct Message {
        QString text;
        MessageType type;
        QDateTime timestamp;
    };
    
    QList<Message> m_messages;
};

#endif // CONSOLEMESSAGEREPOSITORY_H