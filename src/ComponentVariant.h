#ifndef COMPONENTVARIANT_H
#define COMPONENTVARIANT_H

#include "Frame.h"
#include <QString>

class ComponentVariant : public Frame
{
    Q_OBJECT

public:
    explicit ComponentVariant(const QString &id, QObject *parent = nullptr);
    ~ComponentVariant();

signals:

private:

};

#endif // COMPONENTVARIANT_H