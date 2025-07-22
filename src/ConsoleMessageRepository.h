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
    Q_PROPERTY(bool isUsingAI READ isUsingAI WRITE setIsUsingAI NOTIFY isUsingAIChanged)
    Q_PROPERTY(int selectedOption READ selectedOption WRITE setSelectedOption NOTIFY selectedOptionChanged)
    Q_PROPERTY(bool showAIPrompt READ showAIPrompt WRITE setShowAIPrompt NOTIFY showAIPromptChanged)
    
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
    
    // Message with ID support for updates
    QString addMessageWithId(const QString &text, MessageType type = Output);
    void updateMessage(const QString &id, const QString &text);
    void removeMessage(const QString &id);
    
    // AI state
    bool isUsingAI() const;
    void setIsUsingAI(bool using_ai);
    
    // Option selection (0 = Accept, 1 = Feedback)
    int selectedOption() const;
    void setSelectedOption(int option);
    
    // AI prompt visibility
    bool showAIPrompt() const;
    void setShowAIPrompt(bool show);
    
signals:
    void messagesChanged();
    void messageAdded(const QString &text, MessageType type);
    void aiCommandReceived(const QString& prompt);
    void isUsingAIChanged();
    void selectedOptionChanged();
    void showAIPromptChanged();
    void aiContinuationResponse(bool accepted, const QString& feedback);
    
private:
    static ConsoleMessageRepository* s_instance;
    
    struct Message {
        QString id;
        QString text;
        MessageType type;
        QDateTime timestamp;
    };
    
    QList<Message> m_messages;
    bool m_isUsingAI;
    int m_selectedOption;
    bool m_showAIPrompt;
};

#endif // CONSOLEMESSAGEREPOSITORY_H