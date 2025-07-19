#include "ConsoleMessageRepository.h"
#include <QDebug>

ConsoleMessageRepository* ConsoleMessageRepository::s_instance = nullptr;

ConsoleMessageRepository::ConsoleMessageRepository(QObject *parent)
    : QObject(parent)
{
    if (!s_instance) {
        s_instance = this;
    }
}

ConsoleMessageRepository* ConsoleMessageRepository::instance()
{
    if (!s_instance) {
        s_instance = new ConsoleMessageRepository();
    }
    return s_instance;
}

QVariantList ConsoleMessageRepository::messages() const
{
    QVariantList result;
    for (const auto &msg : m_messages) {
        QVariantMap messageMap;
        messageMap["text"] = msg.text;
        messageMap["type"] = messageTypeToString(msg.type);
        messageMap["timestamp"] = msg.timestamp.toString("hh:mm:ss");
        result.append(messageMap);
    }
    return result;
}

void ConsoleMessageRepository::addMessage(const QString &text, MessageType type)
{
    Message msg;
    msg.text = text;
    msg.type = type;
    msg.timestamp = QDateTime::currentDateTime();
    
    m_messages.append(msg);
    
    // Log to debug console as well
    switch (type) {
    case Error:
        qCritical() << "[Console]" << text;
        break;
    case Warning:
        qWarning() << "[Console]" << text;
        break;
    default:
        qDebug() << "[Console]" << text;
        break;
    }
    
    emit messageAdded(text, type);
    emit messagesChanged();
}

void ConsoleMessageRepository::clearMessages()
{
    m_messages.clear();
    emit messagesChanged();
}

QString ConsoleMessageRepository::messageTypeToString(MessageType type) const
{
    switch (type) {
    case Input:
        return "input";
    case Output:
        return "output";
    case Error:
        return "error";
    case Warning:
        return "warning";
    case Info:
        return "info";
    default:
        return "output";
    }
}

void ConsoleMessageRepository::processConsoleCommand(const QString& command) {
    // Add the command to console as input
    addInput(command);
    
    // Check if it's a /cubitAI command
    if (command.startsWith("/cubitAI ", Qt::CaseInsensitive)) {
        // Extract the prompt after /cubitAI
        QString prompt = command.mid(9).trimmed(); // 9 is length of "/cubitAI "
        
        if (prompt.isEmpty()) {
            addError("Usage: /cubitAI <prompt>");
            return;
        }
        
        // Emit signal for Application to handle the CubitAI request
        emit cubitAICommandReceived(prompt);
    } else {
        // Unknown command
        addOutput("Unknown command: " + command);
        addInfo("Available commands: /cubitAI <prompt>");
    }
}