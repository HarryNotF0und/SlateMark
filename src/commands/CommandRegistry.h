#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <functional>

struct AppCommand
{
    QString id;
    QString title;
    QString shortcut;
    std::function<void()> handler;
};

class CommandRegistry : public QObject
{
    Q_OBJECT

public:
    explicit CommandRegistry(QObject* parent = nullptr);
    void add(const AppCommand& command);
    QVector<AppCommand> commands() const;
    bool run(const QString& id) const;

private:
    QVector<AppCommand> m_commands;
};
