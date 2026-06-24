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

void PreviewWidget::setEngine(PreviewEngine engine)
{
#if !defined(SLATEMARK_HAS_WEBENGINE)
    engine = PreviewEngine::Lightweight;
#endif
    if (m_engine == engine) {
        return;
    }
    m_engine = engine;
    releaseResources();
    if (isVisible() && !m_pendingMarkdown.isEmpty()) {
        renderNow();
    }
}

void PreviewWidget::ensureLightweightView()
{
    if (m_textView) {
        return;
    }
    m_textView = new QTextBrowser(this);
    m_textView->setOpenExternalLinks(false);
    m_textView->setOpenLinks(false);
    m_textView->document()->setDocumentMargin(0);
    m_layout->addWidget(m_textView);
    connect(m_textView, &QTextBrowser::anchorClicked, this, [](const QUrl& url) {
        if (url.scheme() == QStringLiteral("http") || url.scheme() == QStringLiteral("https") || url.scheme() == QStringLiteral("mailto")) {
            QDesktopServices::openUrl(url);
        }
    });
    connect(m_textView->verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int value) {
        const int max = m_textView->verticalScrollBar()->maximum();
        emit scrollRatioChanged(max <= 0 ? 0.0 : static_cast<double>(value) / max);
    });
    m_textView->show();
}

void PreviewWidget::ensureView()
{
    if (m_engine == PreviewEngine::Lightweight) {
        ensureLightweightView();
        return;
    }

#if defined(SLATEMARK_HAS_WEBENGINE)
    if (m_webView) {
        return;
    }

    m_webView = new QWebEngineView(this);
    m_profile = new QWebEngineProfile(m_webView);
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

    m_webView->setPage(new SafePreviewPage(m_profile, m_webView));
    QWebEngineSettings* settings = m_webView->settings();
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
    m_bridge = new PreviewBridge(m_webView);
    auto* channel = new QWebChannel(m_webView);
    channel->registerObject(QStringLiteral("bridge"), m_bridge);
    m_webView->page()->setWebChannel(channel);
#endif
    m_layout->addWidget(m_webView);
#if defined(SLATEMARK_HAS_WEBCHANNEL)
    connect(m_bridge, &PreviewBridge::sourceLineClicked, this, &PreviewWidget::sourceLineClicked);
    connect(m_bridge, &PreviewBridge::scrollRatioChanged, this, &PreviewWidget::scrollRatioChanged);
#endif
    m_webView->show();
#else
    ensureLightweightView();
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
    if (m_textView
#if defined(SLATEMARK_HAS_WEBENGINE)
        || m_webView
#endif
    ) {
        renderNow();
    }
}

void PreviewWidget::scrollToRatio(double ratio)
{
    const double clamped = qBound(0.0, ratio, 1.0);
    if (m_engine == PreviewEngine::Lightweight) {
        if (!m_textView) {
            return;
        }
        QScrollBar* bar = m_textView->verticalScrollBar();
        bar->setValue(qRound(clamped * bar->maximum()));
        return;
    }

#if defined(SLATEMARK_HAS_WEBENGINE)
    if (!m_webView) {
        return;
    }
    m_webView->page()->runJavaScript(QStringLiteral("window.__slateMarkScrollToRatio(%1);").arg(clamped, 0, 'f', 4));
#endif
}

void PreviewWidget::releaseLightweightView()
{
    if (!m_textView) {
        return;
    }
    m_layout->removeWidget(m_textView);
    m_textView->deleteLater();
    m_textView = nullptr;
}

void PreviewWidget::releaseWebEngineView()
{
#if defined(SLATEMARK_HAS_WEBENGINE)
    if (!m_webView) {
        return;
    }
#if defined(SLATEMARK_HAS_WEBCHANNEL)
    m_bridge = nullptr;
#endif
    m_webView->setHtml(QString());
    if (m_profile) {
        m_profile->clearHttpCache();
    }
    m_layout->removeWidget(m_webView);
    m_webView->deleteLater();
    m_webView = nullptr;
    m_profile = nullptr;
#endif
}

void PreviewWidget::releaseResources()
{
    m_debounce.stop();
    releaseLightweightView();
    releaseWebEngineView();
}

void PreviewWidget::renderNow()
{
    ensureView();
    const QString html = wrapHtml(MarkdownService::markdownToHtml(m_pendingMarkdown), m_engine == PreviewEngine::WebEngine);
    if (m_engine == PreviewEngine::Lightweight) {
        m_textView->setHtml(html);
        return;
    }

#if defined(SLATEMARK_HAS_WEBENGINE)
    m_webView->setHtml(html, QUrl(QStringLiteral("qrc:/preview/")));
#else
    m_textView->setHtml(html);
#endif
}

QString PreviewWidget::wrapHtml(const QString& body, bool includeScript) const
{
    const QString css = readResource(QStringLiteral(":/preview/preview.css"));
    const QString js = includeScript ? readResource(QStringLiteral(":/preview/preview.js")) : QString();
    const QString script = includeScript
        ? QStringLiteral("<script src=\"qrc:///qtwebchannel/qwebchannel.js\"></script><script>%1</script>").arg(js)
        : QString();
    return QStringLiteral("<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
                          "<style>%1</style></head><body class=\"%2\"><main class=\"markdown-body\">%3</main>%4</body></html>")
        .arg(css, m_dark ? QStringLiteral("dark") : QStringLiteral("light"), body, script);
}
