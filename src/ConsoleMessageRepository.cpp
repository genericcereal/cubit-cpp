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