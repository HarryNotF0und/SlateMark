#include "commands/CommandRegistry.h"

CommandRegistry::CommandRegistry(QObject* parent)
    : QObject(parent)
{
}

void CommandRegistry::add(const AppCommand& command)
{
    m_commands.push_back(command);
}

QVector<AppCommand> CommandRegistry::commands() const
{
    return m_commands;
}

bool CommandRegistry::run(const QString& id) const
{
    for (const AppCommand& command : m_commands) {
        if (command.id == id && command.handler) {
            command.handler();
            return true;
        }
    }
    return false;
}

