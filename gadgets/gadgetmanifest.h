#ifndef GADGETMANIFEST_H
#define GADGETMANIFEST_H

#include <QObject>
#include <QUrl>
#include <QString>
#include <QList>

class QNetworkReply;
class Environment;

/**
 * <Module>
 *  <ModulePrefs title="Are You In?" height="100" width="500">
 *   <Require feature="opensocial-templates"/>
 *   <Require feature="opensocial-data"/>
 *   <Require feature="dynamic-height"/>
 *  </ModulePrefs>
 *  <Content type="html">
 *  </Content>
 * </Module>
 */
class GadgetManifest : public QObject
{
    Q_OBJECT
public:
    GadgetManifest(const QUrl& url, Environment* environment, QObject* parent = 0);
    ~GadgetManifest();

    void load();

    bool isMalformed() const { return m_malformed; }
    QUrl url() const { return m_url; }
    QString content() const { return m_content; }
    QString contentType() const { return m_contentType; }
    int width() const { return m_width; }
    int height() const { return m_height; }
    QString title() const { return m_title; }
    QList<QString> requiredFeatures() const { return m_requiredFeatures; }

signals:
    void finished();

private slots:
    void parse();

private:
    QUrl m_url;
    QNetworkReply* m_reply;
    bool m_malformed;
    QString m_title;
    int m_height;
    int m_width;
    QList<QString> m_requiredFeatures;
    QString m_content;
    QString m_contentType;
    Environment* m_environment;
};

#endif // GADGETMANIFEST_H
