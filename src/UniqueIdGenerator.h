#pragma once

#include <QString>
#include <QUuid>
#include <random>

class UniqueIdGenerator {
public:
    static QString generate16DigitId() {
        static std::random_device rd;
        static std::mt19937_64 gen(rd());
        static std::uniform_int_distribution<uint64_t> dis(1000000000000000ULL, 9999999999999999ULL);
        
        return QString::number(dis(gen));
    }
    
    static QString generateUuid() {
        return QUuid::createUuid().toString(QUuid::Id128);
    }
};