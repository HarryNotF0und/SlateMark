#pragma once

#include "preview/PreviewBridge.h"

#include <QTimer>
#include <QUrl>
#include <QWidget>

class QVBoxLayout;

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
#else
class QTextBrowser;
#endif

class PreviewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PreviewWidget(QWidget* parent = nullptr);
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
    QString wrapHtml(const QString& body) const;

    QVBoxLayout* m_layout = nullptr;
#if defined(SLATEMARK_HAS_WEBENGINE)
    QWebEngineView* m_view = nullptr;
    QWebEngineProfile* m_profile = nullptr;
#if defined(SLATEMARK_HAS_WEBCHANNEL)
    PreviewBridge* m_bridge = nullptr;
#endif
#else
    QTextBrowser* m_view = nullptr;
#endif
    QTimer m_debounce;
    QString m_pendingMarkdown;
    bool m_dark = true;
};
