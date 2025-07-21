#include "ConsoleMessageRepository.h"
#include <QDebug>
#include <QUuid>

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
        messageMap["id"] = msg.id;
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
    msg.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
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
    
    // Check if it's an /ai command
    if (command.startsWith("/ai ", Qt::CaseInsensitive)) {
        // Extract the prompt after /ai
        QString prompt = command.mid(4).trimmed(); // 4 is length of "/ai "
        
        if (prompt.isEmpty()) {
            addError("Usage: /ai <prompt>");
            return;
        }
        
        // Emit signal for Application to handle the AI request
        emit aiCommandReceived(prompt);
    } else {
        // Unknown command
        addOutput("Unknown command: " + command);
        addInfo("Available commands: /ai <prompt>");
    }
}

QString ConsoleMessageRepository::addMessageWithId(const QString &text, MessageType type)
{
    Message msg;
    msg.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
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
    
    return msg.id;
}

void ConsoleMessageRepository::updateMessage(const QString &id, const QString &text)
{
    for (int i = 0; i < m_messages.size(); ++i) {
        if (m_messages[i].id == id) {
            m_messages[i].text = text;
            emit messagesChanged();
            break;
        }
    }
}

void ConsoleMessageRepository::removeMessage(const QString &id)
{
    m_messages.removeIf([&id](const Message &msg) {
        return msg.id == id;
    });
    emit messagesChanged();
}