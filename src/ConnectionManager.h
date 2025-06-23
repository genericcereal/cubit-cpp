#pragma once
#include <QMetaObject>
#include <QVector>

class ConnectionManager {
public:
    ConnectionManager() = default;
    ~ConnectionManager() {
        for (auto& c : m_connections) {
            QObject::disconnect(c);
        }
    }
    
    void add(const QMetaObject::Connection& c) {
        if (c) {
            m_connections.push_back(c);
        }
    }
    
    void clear() {
        for (auto& c : m_connections) {
            QObject::disconnect(c);
        }
        m_connections.clear();
    }
    
private:
    QVector<QMetaObject::Connection> m_connections;
};