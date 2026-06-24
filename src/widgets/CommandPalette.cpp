#include "widgets/CommandPalette.h"

#include <QKeyEvent>
#include <QListWidgetItem>
#include <QVBoxLayout>

CommandPalette::CommandPalette(CommandRegistry* registry, QWidget* parent)
    : QDialog(parent)
    , m_registry(registry)
{
    setObjectName(QStringLiteral("commandPalette"));
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    resize(520, 420);
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    m_search = new QLineEdit(this);
    m_search->setPlaceholderText(QStringLiteral("Search commands"));
    m_list = new QListWidget(this);
    layout->addWidget(m_search);
    layout->addWidget(m_list);
    connect(m_search, &QLineEdit::textChanged, this, &CommandPalette::refresh);
    connect(m_list, &QListWidget::itemActivated, this, [this](QListWidgetItem* item) {
        if (!item) return;
        m_registry->run(item->data(Qt::UserRole).toString());
        close();
    });
}

void CommandPalette::openPalette()
{
    refresh();
    show();
    raise();
    activateWindow();
    m_search->setFocus();
    m_search->selectAll();
}

void CommandPalette::refresh()
{
    m_list->clear();
    const QString query = m_search->text().trimmed();
    for (const AppCommand& command : m_registry->commands()) {
        if (!query.isEmpty() && !command.title.contains(query, Qt::CaseInsensitive)) {
            continue;
        }
        auto* item = new QListWidgetItem(command.shortcut.isEmpty()
                ? command.title
                : QStringLiteral("%1    %2").arg(command.title, command.shortcut), m_list);
        item->setData(Qt::UserRole, command.id);
    }
    if (m_list->count() > 0) {
        m_list->setCurrentRow(0);
    }
}
