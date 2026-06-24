#pragma once

#include "commands/CommandRegistry.h"

#include <QDialog>
#include <QLineEdit>
#include <QListWidget>

class CommandPalette : public QDialog
{
    Q_OBJECT

public:
    explicit CommandPalette(CommandRegistry* registry, QWidget* parent = nullptr);
    void openPalette();

private:
    void refresh();

    CommandRegistry* m_registry = nullptr;
    QLineEdit* m_search = nullptr;
    QListWidget* m_list = nullptr;
};

