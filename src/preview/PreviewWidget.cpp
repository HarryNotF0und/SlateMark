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
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#endif

#if defined(SLATEMARK_HAS_WEBENGINE)
SafePreviewPage::SafePreviewPage(QWebEngineProfile* profile, QObject* parent)
    : QWebEnginePage(profile, parent)
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
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);

    m_debounce.setSingleShot(true);
    m_debounce.setInterval(180);
    connect(&m_debounce, &QTimer::timeout, this, &PreviewWidget::renderNow);
}

void PreviewWidget::ensureView()
{
    if (m_view) {
        return;
    }

#if defined(SLATEMARK_HAS_WEBENGINE)
    m_view = new QWebEngineView(this);
    m_profile = new QWebEngineProfile(m_view);
    m_profile->setHttpCacheType(QWebEngineProfile::NoCache);
    m_profile->setHttpCacheMaximumSize(0);
    m_profile->setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);
    m_profile->setPersistentPermissionsPolicy(QWebEngineProfile::PersistentPermissionsPolicy::StoreInMemory);
    m_profile->setSpellCheckEnabled(false);
    m_profile->settings()->setAttribute(QWebEngineSettings::AutoLoadIconsForPage, false);
    m_profile->settings()->setAttribute(QWebEngineSettings::TouchIconsEnabled, false);
    m_profile->settings()->setAttribute(QWebEngineSettings::DnsPrefetchEnabled, false);
    m_profile->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, false);
    m_profile->settings()->setAttribute(QWebEngineSettings::PdfViewerEnabled, false);
    m_profile->settings()->setAttribute(QWebEngineSettings::WebGLEnabled, false);
    m_profile->settings()->setAttribute(QWebEngineSettings::Accelerated2dCanvasEnabled, false);
    m_profile->settings()->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled, false);
    m_profile->settings()->setAttribute(QWebEngineSettings::BackForwardCacheEnabled, false);

    m_view->setPage(new SafePreviewPage(m_profile, m_view));
    QWebEngineSettings* settings = m_view->settings();
    settings->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, false);
    settings->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, false);
    settings->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, false);
    settings->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, false);
    settings->setAttribute(QWebEngineSettings::JavascriptCanPaste, false);
    settings->setAttribute(QWebEngineSettings::LocalStorageEnabled, false);
    settings->setAttribute(QWebEngineSettings::HyperlinkAuditingEnabled, false);
    settings->setAttribute(QWebEngineSettings::PluginsEnabled, false);
    settings->setAttribute(QWebEngineSettings::PdfViewerEnabled, false);
    settings->setAttribute(QWebEngineSettings::WebGLEnabled, false);
    settings->setAttribute(QWebEngineSettings::Accelerated2dCanvasEnabled, false);
    settings->setAttribute(QWebEngineSettings::AutoLoadIconsForPage, false);
    settings->setAttribute(QWebEngineSettings::TouchIconsEnabled, false);
    settings->setAttribute(QWebEngineSettings::DnsPrefetchEnabled, false);
    settings->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled, false);
    settings->setAttribute(QWebEngineSettings::BackForwardCacheEnabled, false);
#if defined(SLATEMARK_HAS_WEBCHANNEL)
    m_bridge = new PreviewBridge(m_view);
    auto* channel = new QWebChannel(m_view);
    channel->registerObject(QStringLiteral("bridge"), m_bridge);
    m_view->page()->setWebChannel(channel);
#endif
    m_layout->addWidget(m_view);
#if defined(SLATEMARK_HAS_WEBCHANNEL)
    connect(m_bridge, &PreviewBridge::sourceLineClicked, this, &PreviewWidget::sourceLineClicked);
    connect(m_bridge, &PreviewBridge::scrollRatioChanged, this, &PreviewWidget::scrollRatioChanged);
#endif
#else
    m_view = new QTextBrowser(this);
    m_view->setOpenExternalLinks(false);
    m_view->setOpenLinks(false);
    m_layout->addWidget(m_view);
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
    m_view->show();
}

void PreviewWidget::setMarkdown(const QString& markdown)
{
    m_pendingMarkdown = markdown;
    m_debounce.start();
}

void PreviewWidget::setDarkTheme(bool dark)
{
    m_dark = dark;
    if (m_view) {
        renderNow();
    }
}

void PreviewWidget::scrollToRatio(double ratio)
{
    if (!m_view) {
        return;
    }
    const double clamped = qBound(0.0, ratio, 1.0);
#if defined(SLATEMARK_HAS_WEBENGINE)
    m_view->page()->runJavaScript(QStringLiteral("window.__slateMarkScrollToRatio(%1);").arg(clamped, 0, 'f', 4));
#else
    QScrollBar* bar = m_view->verticalScrollBar();
    bar->setValue(qRound(clamped * bar->maximum()));
#endif
}

void PreviewWidget::releaseResources()
{
    m_debounce.stop();
    if (!m_view) {
        return;
    }
#if defined(SLATEMARK_HAS_WEBCHANNEL)
    m_bridge = nullptr;
#endif
#if defined(SLATEMARK_HAS_WEBENGINE)
    m_view->setHtml(QString());
    if (m_profile) {
        m_profile->clearHttpCache();
    }
#endif
    m_layout->removeWidget(m_view);
    m_view->deleteLater();
    m_view = nullptr;
#if defined(SLATEMARK_HAS_WEBENGINE)
    m_profile = nullptr;
#endif
}

void PreviewWidget::renderNow()
{
    ensureView();
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
