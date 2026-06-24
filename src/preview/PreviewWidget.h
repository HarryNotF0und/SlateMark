#pragma once

#include "preview/PreviewBridge.h"

#include <QTimer>
#include <QUrl>
#include <QWidget>

class QTextBrowser;
class QVBoxLayout;

enum class PreviewEngine
{
    Lightweight,
    WebEngine
};

#if defined(SLATEMARK_HAS_WEBENGINE)
#include <QWebEnginePage>
#include <QWebEngineView>

class QWebEngineProfile;

class SafePreviewPage : public QWebEnginePage
{
    Q_OBJECT

public:
    explicit SafePreviewPage(QWebEngineProfile* profile, QObject* parent = nullptr);

protected:
    bool acceptNavigationRequest(const QUrl& url, NavigationType type, bool isMainFrame) override;
};
#endif

class PreviewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PreviewWidget(QWidget* parent = nullptr);
    void setEngine(PreviewEngine engine);
    void setMarkdown(const QString& markdown);
    void setDarkTheme(bool dark);
    void scrollToRatio(double ratio);
    void releaseResources();

signals:
    void sourceLineClicked(int line);
    void scrollRatioChanged(double ratio);

private slots:
    void renderNow();

private:
    void ensureView();
    void ensureLightweightView();
    void releaseLightweightView();
    void releaseWebEngineView();
    QString wrapHtml(const QString& body, bool includeScript) const;

    QVBoxLayout* m_layout = nullptr;
    QTextBrowser* m_textView = nullptr;
#if defined(SLATEMARK_HAS_WEBENGINE)
    QWebEngineView* m_webView = nullptr;
    QWebEngineProfile* m_profile = nullptr;
#if defined(SLATEMARK_HAS_WEBCHANNEL)
    PreviewBridge* m_bridge = nullptr;
#endif
#endif
    QTimer m_debounce;
    QString m_pendingMarkdown;
    PreviewEngine m_engine = PreviewEngine::Lightweight;
    bool m_dark = true;
};
