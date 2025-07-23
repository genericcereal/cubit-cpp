#pragma once

#include <QObject>
#include <QJsonArray>
#include <QTimer>
#include <QWebSocket>
#include <memory>

class AuthenticationManager;
class Application;
class AICommandDispatcher;

/**
 * StreamingAIClient handles:
 *  - WebSocket connection to AppSync realtime API
 *  - Creating conversations
 *  - Sending user messages
 *  - Subscribing to assistant response stream
 *  - Processing AI responses streamed from backend Lambda
 */
class StreamingAIClient : public QObject
{
    Q_OBJECT

public:
    explicit StreamingAIClient(AuthenticationManager *authManager, Application *app, QObject *parent = nullptr);
    ~StreamingAIClient() override;

    /** Send a user message and initiate AI response */
    void sendMessage(const QString &description);

    /** Returns true if WebSocket connected + connection_ack received */
    bool isConnected() const;
    
    /** Handle user response to AI continuation prompt */
    void handleUserContinuationResponse(bool accepted, const QString &feedback = QString());
    
    /** Clear the current conversation (useful when AI mode is toggled off) */
    void clearConversation();

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString &error);
    void responseChunkReceived(const QString &chunk);
    void responseReceived(const QString &fullResponse);
    void commandsReceived(const QJsonArray &commands);
    void streamingStarted();
    void streamingCompleted();

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);
    void onError(QAbstractSocket::SocketError error);
    void onTokensRefreshed();
    void sendPing();

private:
    // ==== Connection + Auth ====
    void connectToWebSocket();
    void disconnectFromWebSocket();
    QString getWebSocketUrl() const;
    QString getAuthToken() const;
    void sendConnectionInit();
    void handleConnectionAck();

    // ==== Conversation Flow ====
    void createConversation();
    void subscribeToResponses(const QString &conversationId);
    void sendSubscription(const QString &subscriptionId,
                          const QString &query,
                          const QJsonObject &variables);

    void sendConversationMessage(const QString &conversationId,
                                 const QString &message);
    void sendCubitAIRules(const QString &conversationId);

    // ==== Subscription Handling ====
    void handleSubscriptionData(const QJsonObject &payload);
    void processStreamingResponse(const QString &chunk,
                                  int blockIndex,
                                  bool isComplete);
    void finalizeResponse();
    void handleComplete(const QString &subscriptionId);
    void handleError(const QJsonObject &payload);

    // ==== Canvas ====
    QString getCanvasState() const;
    
    // ==== Loading Indicator ====
    void startLoadingIndicator();
    void stopLoadingIndicator();
    void updateLoadingIndicator();
    
    // ==== JSON Repair ====
    void requestJsonRepair(const QString &invalidJson, const QString &errorMessage);
    
    // ==== Plan Parsing ====
    QStringList extractPlanSteps(const QString &aiResponse);
    void executeNextPlanStep();
    
    // ==== Tool Registry ====
    void fetchToolRegistry(const QString &category);

private:
    std::unique_ptr<QWebSocket> m_webSocket;
    AuthenticationManager *m_authManager;
    Application *m_application;
    std::unique_ptr<AICommandDispatcher> m_commandDispatcher;

    QTimer *m_pingTimer;
    bool m_isConnected;
    bool m_connectionAckReceived;
    int m_reconnectAttempts;

    QString m_pendingMessage;
    QString m_accumulatedResponse;
    QString m_accumulatedCommands;
    QString m_currentConversationId;
    QString m_currentSubscriptionId;
    int m_continuationCount;
    QString m_pendingContinuationContext;
    
    // Loading indicator
    QTimer *m_loadingTimer;
    int m_loadingDots;
    QString m_loadingMessageId;
    
    // Plan execution
    QStringList m_pendingPlanSteps;   // parsed PLAN steps
    int m_currentStepIndex = 0;       // which step we're on

    // Constants
    static constexpr int PING_INTERVAL_MS = 300000; // AWS default keepalive 5 min
    static constexpr int MAX_RECONNECT_ATTEMPTS = 3;
    static constexpr int LOADING_ANIMATION_MS = 500; // Update dots every 500ms
};
