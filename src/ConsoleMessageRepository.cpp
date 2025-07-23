#include "ConsoleMessageRepository.h"
#include <QDebug>
#include <QUuid>

ConsoleMessageRepository* ConsoleMessageRepository::s_instance = nullptr;

ConsoleMessageRepository::ConsoleMessageRepository(QObject *parent)
    : QObject(parent), m_isUsingAI(false), m_selectedOption(1), m_showAIPrompt(false)
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
    
    // Check if it's an /ai command to toggle AI mode
    if (command.trimmed().toLower() == "/ai") {
        // Toggle AI mode
        setIsUsingAI(!m_isUsingAI);
        
        if (m_isUsingAI) {
            addInfo("AI mode enabled. Initializing AI assistant...");
            // Send initialization message to trigger AI initialization with rules
            emit aiCommandReceived("INIT_AI_WITH_RULES");
        } else {
            addInfo("AI mode disabled.");
            emit aiModeDisabled();
        }
        return;
    }
    
    // If AI mode is enabled, treat all input as AI prompts
    if (m_isUsingAI) {
        // Check if we're showing the AI prompt (user needs to respond to continuation)
        if (m_showAIPrompt) {
            // Handle the user's response based on selected option
            if (m_selectedOption == 0 && command.isEmpty()) {
                // User selected "Accept" and pressed Enter without typing anything
                emit aiContinuationResponse(true, QString());
                // Hide the prompt after handling the response
                setShowAIPrompt(false);
            } else if (m_selectedOption == 1 && !command.isEmpty()) {
                // User selected "Feedback" and provided custom feedback
                emit aiContinuationResponse(false, command);
                // Hide the prompt after handling the response
                setShowAIPrompt(false);
            }
            // If "Feedback" is selected but command is empty, do nothing (wait for actual feedback)
        } else {
            // Normal AI command - but skip empty commands
            if (!command.isEmpty()) {
                emit aiCommandReceived(command);
            }
        }
    } else if (command.startsWith("/")) {
        // It's a command but not /ai
        addOutput("Unknown command: " + command);
        addInfo("Available commands: /ai (toggle AI mode)");
    } else {
        // Regular console message when not in AI mode
        addOutput("Type /ai to enable AI mode");
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

bool ConsoleMessageRepository::isUsingAI() const
{
    return m_isUsingAI;
}

void ConsoleMessageRepository::setIsUsingAI(bool using_ai)
{
    if (m_isUsingAI != using_ai) {
        m_isUsingAI = using_ai;
        emit isUsingAIChanged();
    }
}

int ConsoleMessageRepository::selectedOption() const
{
    return m_selectedOption;
}

void ConsoleMessageRepository::setSelectedOption(int option)
{
    if (m_selectedOption != option && option >= 0 && option <= 1) {
        m_selectedOption = option;
        emit selectedOptionChanged();
    }
}

bool ConsoleMessageRepository::showAIPrompt() const
{
    return m_showAIPrompt;
}

void ConsoleMessageRepository::setShowAIPrompt(bool show)
{
    if (m_showAIPrompt != show) {
        m_showAIPrompt = show;
        emit showAIPromptChanged();
    }
}