#include "widgets/ToolPanel.h"

#include "widgets/SvgIconButton.h"

#include <QGridLayout>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QToolButton>
#include <QVBoxLayout>

ToolPanel::ToolPanel(ThemeManager* theme, QWidget* parent)
    : QWidget(parent)
    , m_theme(theme)
{
    setObjectName(QStringLiteral("toolPanel"));
    setFixedWidth(248);
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(14, 14, 14, 14);
    root->setSpacing(12);

    auto* title = new QLabel(QStringLiteral("TOOLS<br><span>Markdown tools</span>"), this);
    title->setObjectName(QStringLiteral("panelTitle"));
    title->setTextFormat(Qt::RichText);
    root->addWidget(title);

    auto* scroll = new QScrollArea(this);
    scroll->setObjectName(QStringLiteral("toolPanelScroll"));
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->viewport()->setObjectName(QStringLiteral("toolPanelViewport"));
    auto* content = new QWidget(scroll);
    content->setObjectName(QStringLiteral("toolPanelContent"));
    auto* contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(14);
    scroll->setWidget(content);
    root->addWidget(scroll, 1);

    contentLayout->addWidget(createSection(QStringLiteral("TEXT"), {
        {QStringLiteral("bold"), QStringLiteral("bold"), QStringLiteral("Bold")},
        {QStringLiteral("italic"), QStringLiteral("italic"), QStringLiteral("Italic")},
        {QStringLiteral("strikethrough"), QStringLiteral("strikethrough"), QStringLiteral("Strikethrough")},
        {QStringLiteral("inline-code"), QStringLiteral("inline-code"), QStringLiteral("Inline code")},
        {QStringLiteral("link"), QStringLiteral("link"), QStringLiteral("Insert link")},
        {QStringLiteral("image"), QStringLiteral("image"), QStringLiteral("Insert image")},
        {QStringLiteral("quote"), QStringLiteral("quote"), QStringLiteral("Block quote")}
    }));

    auto* headings = new QWidget(this);
    auto* hLayout = new QGridLayout(headings);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(8);
    auto* hTitle = new QLabel(QStringLiteral("HEADINGS"), this);
    hTitle->setObjectName(QStringLiteral("sectionLabel"));
    contentLayout->addWidget(hTitle);
    for (int i = 0; i <= 6; ++i) {
        const QString icon = i == 0 ? QStringLiteral("paragraph") : QStringLiteral("heading-%1").arg(i);
        auto* button = new SvgIconButton(icon, m_theme, headings);
        button->setObjectName(QStringLiteral("panelIconButton"));
        button->setToolTip(i == 0 ? QStringLiteral("Normal text") : QStringLiteral("Heading %1").arg(i));
        hLayout->addWidget(button, i / 3, i % 3, Qt::AlignCenter);
        connect(button, &QToolButton::clicked, this, [this, i] { emit headingRequested(i); });
    }
    contentLayout->addWidget(headings);

    contentLayout->addWidget(createSection(QStringLiteral("STRUCTURE"), {
        {QStringLiteral("unordered-list"), QStringLiteral("unordered-list"), QStringLiteral("Bullet list")},
        {QStringLiteral("ordered-list"), QStringLiteral("ordered-list"), QStringLiteral("Numbered list")},
        {QStringLiteral("task-list"), QStringLiteral("task-list"), QStringLiteral("Task list")},
        {QStringLiteral("indent"), QStringLiteral("indent"), QStringLiteral("Indent")},
        {QStringLiteral("outdent"), QStringLiteral("outdent"), QStringLiteral("Outdent")}
    }));

    contentLayout->addWidget(createSection(QStringLiteral("BLOCKS"), {
        {QStringLiteral("code-block"), QStringLiteral("code-block"), QStringLiteral("Code block")},
        {QStringLiteral("table"), QStringLiteral("table"), QStringLiteral("Insert table")},
        {QStringLiteral("horizontal-rule"), QStringLiteral("horizontal-rule"), QStringLiteral("Horizontal rule")},
        {QStringLiteral("details"), QStringLiteral("details"), QStringLiteral("Details block")},
        {QStringLiteral("math"), QStringLiteral("math"), QStringLiteral("Math block")},
        {QStringLiteral("footnote"), QStringLiteral("footnote"), QStringLiteral("Footnote")},
        {QStringLiteral("mermaid"), QStringLiteral("mermaid"), QStringLiteral("Mermaid diagram")},
        {QStringLiteral("quote"), QStringLiteral("quote"), QStringLiteral("Block quote")}
    }));

    auto* statsTitle = new QLabel(QStringLiteral("DOCUMENT STATS"), this);
    statsTitle->setObjectName(QStringLiteral("sectionLabel"));
    contentLayout->addWidget(statsTitle);
    m_stats = new QLabel(this);
    m_stats->setObjectName(QStringLiteral("statsLabel"));
    contentLayout->addWidget(m_stats);

    auto* outlineTitle = new QLabel(QStringLiteral("OUTLINE"), this);
    outlineTitle->setObjectName(QStringLiteral("sectionLabel"));
    contentLayout->addWidget(outlineTitle);
    auto* outlineHost = new QWidget(this);
    m_outlineLayout = new QVBoxLayout(outlineHost);
    m_outlineLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->addWidget(outlineHost);
    contentLayout->addStretch();
}

QWidget* ToolPanel::createSection(const QString& title, const QVector<ActionSpec>& actions)
{
    auto* section = new QWidget(this);
    auto* layout = new QVBoxLayout(section);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);
    auto* label = new QLabel(title, section);
    label->setObjectName(QStringLiteral("sectionLabel"));
    layout->addWidget(label);
    auto* grid = new QGridLayout;
    grid->setSpacing(8);
    layout->addLayout(grid);

    for (int i = 0; i < actions.size(); ++i) {
        auto* button = new SvgIconButton(actions.at(i).icon, m_theme, section);
        button->setObjectName(QStringLiteral("panelIconButton"));
        button->setToolTip(actions.at(i).tooltip);
        grid->addWidget(button, i / 4, i % 4, Qt::AlignCenter);
        const QString id = actions.at(i).id;
        connect(button, &QToolButton::clicked, this, [this, id] {
            if (id == QStringLiteral("bold")) emit formatRequested(QStringLiteral("**"), QStringLiteral("**"), QStringLiteral("bold text"));
            else if (id == QStringLiteral("italic")) emit formatRequested(QStringLiteral("*"), QStringLiteral("*"), QStringLiteral("italic text"));
            else if (id == QStringLiteral("strikethrough")) emit formatRequested(QStringLiteral("~~"), QStringLiteral("~~"), QStringLiteral("deleted text"));
            else if (id == QStringLiteral("inline-code")) emit formatRequested(QStringLiteral("`"), QStringLiteral("`"), QStringLiteral("code"));
            else if (id == QStringLiteral("link")) emit formatRequested(QStringLiteral("["), QStringLiteral("](https://example.com)"), QStringLiteral("link text"));
            else if (id == QStringLiteral("image")) emit blockRequested(QStringLiteral("![Image description](image.png)"));
            else if (id == QStringLiteral("quote")) emit blockRequested(QStringLiteral("> Quote"));
            else if (id == QStringLiteral("unordered-list")) emit blockRequested(QStringLiteral("- List item"));
            else if (id == QStringLiteral("ordered-list")) emit blockRequested(QStringLiteral("1. List item"));
            else if (id == QStringLiteral("task-list")) emit blockRequested(QStringLiteral("- [ ] Task item"));
            else if (id == QStringLiteral("code-block")) emit blockRequested(QStringLiteral("```cpp\n// code\n```"));
            else if (id == QStringLiteral("table")) emit tableRequested();
            else if (id == QStringLiteral("horizontal-rule")) emit blockRequested(QStringLiteral("---"));
            else if (id == QStringLiteral("details")) emit blockRequested(QStringLiteral("<details>\n<summary>Details</summary>\n\nContent\n\n</details>"));
            else if (id == QStringLiteral("math")) emit blockRequested(QStringLiteral("$$\na^2 + b^2 = c^2\n$$"));
            else if (id == QStringLiteral("footnote")) emit blockRequested(QStringLiteral("Footnote reference[^1]\n\n[^1]: Footnote text."));
            else if (id == QStringLiteral("mermaid")) emit blockRequested(QStringLiteral("```mermaid\ngraph TD\n    A --> B\n```"));
        });
    }
    return section;
}

void ToolPanel::setStatistics(const DocumentStatistics& stats)
{
    m_stats->setText(QStringLiteral("Characters: %1\nWords: %2\nLines: %3\nParagraphs: %4\nHeadings: %5\nImages: %6\nLinks: %7\nRead: %8 min")
        .arg(stats.characters)
        .arg(stats.words)
        .arg(stats.lines)
        .arg(stats.paragraphs)
        .arg(stats.headings)
        .arg(stats.images)
        .arg(stats.links)
        .arg(stats.readingMinutes));
}

void ToolPanel::setOutline(const QVector<MarkdownHeading>& headings)
{
    while (QLayoutItem* item = m_outlineLayout->takeAt(0)) {
        delete item->widget();
        delete item;
    }
    for (const MarkdownHeading& heading : headings) {
        auto* button = new QPushButton(QStringLiteral("%1 %2").arg(QString(heading.level - 1, ' '), heading.text), this);
        button->setObjectName(QStringLiteral("outlineButton"));
        button->setToolTip(QStringLiteral("Line %1").arg(heading.line));
        connect(button, &QPushButton::clicked, this, [this, heading] { emit outlineLineRequested(heading.line); });
        m_outlineLayout->addWidget(button);
    }
    m_outlineLayout->addStretch();
}
