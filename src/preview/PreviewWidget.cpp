#include "preview/PreviewWidget.h"

#include "services/MarkdownService.h"

#include <QDesktopServices>
#include <QFile>
#include <QScrollBar>
#include <QTextBrowser>
#include <QUrl>
#include <QVBoxLayout>
#if defined(SLATEMARK_HAS_WEBENGINE) && defined(SLATEMARK_HAS_WEBCHANNEL)
#include <QWebChannel>
#endif
#if defined(SLATEMARK_HAS_WEBENGINE)
#include <QWebEngineSettings>
#endif

#if defined(SLATEMARK_HAS_WEBENGINE)
SafePreviewPage::SafePreviewPage(QObject* parent)
    : QWebEnginePage(parent)
{
}

bool SafePreviewPage::acceptNavigationRequest(const QUrl& url, NavigationType type, bool isMainFrame)
{
    Q_UNUSED(isMainFrame)
    if (type == QWebEnginePage::NavigationTypeLinkClicked) {
        if (url.scheme() == QStringLiteral("http") || url.scheme() == QStringLiteral("https") || url.scheme() == QStringLiteral("mailto")) {
            QDesktopServices::openUrl(url);
        }
        return false;
    }
    if (url.scheme() == QStringLiteral("javascript")) {
        return false;
    }
    return true;
}
#endif

namespace {
QString readResource(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    return QString::fromUtf8(file.readAll());
}
}

PreviewWidget::PreviewWidget(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("previewWidget"));
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
#if defined(SLATEMARK_HAS_WEBENGINE)
    m_view = new QWebEngineView(this);
    m_view->setPage(new SafePreviewPage(m_view));
    m_view->settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, false);
    m_view->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, false);
    m_view->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, false);
#if defined(SLATEMARK_HAS_WEBCHANNEL)
    m_bridge = new PreviewBridge(this);
    auto* channel = new QWebChannel(this);
    channel->registerObject(QStringLiteral("bridge"), m_bridge);
    m_view->page()->setWebChannel(channel);
#endif
    layout->addWidget(m_view);

    m_debounce.setSingleShot(true);
    m_debounce.setInterval(180);
    connect(&m_debounce, &QTimer::timeout, this, &PreviewWidget::renderNow);
#if defined(SLATEMARK_HAS_WEBCHANNEL)
    connect(m_bridge, &PreviewBridge::sourceLineClicked, this, &PreviewWidget::sourceLineClicked);
    connect(m_bridge, &PreviewBridge::scrollRatioChanged, this, &PreviewWidget::scrollRatioChanged);
#endif
#else
    m_view = new QTextBrowser(this);
    m_view->setOpenExternalLinks(false);
    m_view->setOpenLinks(false);
    layout->addWidget(m_view);
    connect(&m_debounce, &QTimer::timeout, this, &PreviewWidget::renderNow);
    connect(m_view, &QTextBrowser::anchorClicked, this, [](const QUrl& url) {
        if (url.scheme() == QStringLiteral("http") || url.scheme() == QStringLiteral("https") || url.scheme() == QStringLiteral("mailto")) {
            QDesktopServices::openUrl(url);
        }
    });
    connect(m_view->verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int value) {
        const int max = m_view->verticalScrollBar()->maximum();
        emit scrollRatioChanged(max <= 0 ? 0.0 : static_cast<double>(value) / max);
    });
#endif
}

void PreviewWidget::setMarkdown(const QString& markdown)
{
    m_pendingMarkdown = markdown;
    m_debounce.start();
}

void PreviewWidget::setDarkTheme(bool dark)
{
    m_dark = dark;
    renderNow();
}

void PreviewWidget::scrollToRatio(double ratio)
{
    const double clamped = qBound(0.0, ratio, 1.0);
#if defined(SLATEMARK_HAS_WEBENGINE)
    m_view->page()->runJavaScript(QStringLiteral("window.__slateMarkScrollToRatio(%1);").arg(clamped, 0, 'f', 4));
#else
    QScrollBar* bar = m_view->verticalScrollBar();
    bar->setValue(qRound(clamped * bar->maximum()));
#endif
}

void PreviewWidget::renderNow()
{
#if defined(SLATEMARK_HAS_WEBENGINE)
    m_view->setHtml(wrapHtml(MarkdownService::markdownToHtml(m_pendingMarkdown)), QUrl(QStringLiteral("qrc:/preview/")));
#else
    m_view->setHtml(wrapHtml(MarkdownService::markdownToHtml(m_pendingMarkdown)));
#endif
}

QString PreviewWidget::wrapHtml(const QString& body) const
{
    const QString css = readResource(QStringLiteral(":/preview/preview.css"));
    const QString js = readResource(QStringLiteral(":/preview/preview.js"));
#if defined(SLATEMARK_HAS_WEBENGINE)
#if defined(SLATEMARK_HAS_WEBCHANNEL)
    return QStringLiteral("<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
                          "<style>%1</style></head><body class=\"%2\"><main class=\"markdown-body\">%3</main>"
                          "<script src=\"qrc:///qtwebchannel/qwebchannel.js\"></script><script>%4</script></body></html>")
        .arg(css, m_dark ? QStringLiteral("dark") : QStringLiteral("light"), body, js);
#else
    Q_UNUSED(js)
    return QStringLiteral("<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
                          "<style>%1</style></head><body class=\"%2\"><main class=\"markdown-body\">%3</main><script>%4</script></body></html>")
        .arg(css, m_dark ? QStringLiteral("dark") : QStringLiteral("light"), body, QStringLiteral("window.__slateMarkScrollToRatio=function(r){var m=document.documentElement.scrollHeight-window.innerHeight;window.scrollTo(0,m*r);};"));
#endif
#else
    Q_UNUSED(js)
    return QStringLiteral("<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><style>%1</style></head><body class=\"%2\"><main class=\"markdown-body\">%3</main></body></html>")
        .arg(css, m_dark ? QStringLiteral("dark") : QStringLiteral("light"), body);
#endif
}
