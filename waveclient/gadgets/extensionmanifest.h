#ifndef EXTENSIONMANIFEST_H
#define EXTENSIONMANIFEST_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QList>

class Environment;
class QNetworkReply;

class ExtensionManifest : public QObject
{
    Q_OBJECT
public:
    class Action
    {
    public:
        enum Type
        {
            InsertGadget = 1
        };

        Action() { }
        Action( const Action& action ) { m_type = action.m_type; m_url = action.m_url; }

        Type type() const { return m_type; }
        QUrl url() const { return m_url; }

    private:
        Type m_type;
        QUrl m_url;
    };

    class Hook
    {
    public:
        enum Location
        {
            Toolbar = 1,
            NewWaveMenu = 2
        };

        Hook() { }
        Hook( const Hook& hook ) { m_location = hook.m_location; m_text = hook.m_text; m_iconUrl = hook.m_iconUrl; m_actions = hook.m_actions; }

        Location location() const { return m_location; }
        void setLocation( Location location ) { m_location = location; }
        QString text() const { return m_text; }
        void setText( const QString& text ) { m_text = text; }
        QUrl iconUrl() const { return m_iconUrl; }
        void setIconUrl( const QUrl& url ) { m_iconUrl = url; }
        QList<Action> actions() const { return m_actions; }
        void appendAction( const Action& action ) { m_actions.append( action ); }

    private:
        Location m_location;
        QString m_text;
        QUrl m_iconUrl;
        QList<Action> m_actions;
    };

    ExtensionManifest(const QUrl& url, Environment* environment);

    void load();

    bool isMalformed() const { return m_malformed; }
    QUrl url() const { return m_url; }
    QString name() const { return m_name; }
    void setName( const QString& name ) { m_name = name; }
    QString description() const { return m_description; }
    void setDescription( const QString& description ) { m_description = description; }
    QString author() const { return m_author; }
    void setAuthor( const QString& author ) { m_author = author; }
    QUrl thumbnailUrl() const { return m_thumbnailUrl; }
    void setThumbnailUrl( const QUrl& url ) { m_thumbnailUrl = url; }
    QList<Hook> hooks() const { return m_hooks; }
    void appendHook( const Hook& hook ) { m_hooks.append(hook); }

signals:
    void finished();

private slots:
    void parse();

private:
    QString m_name;
    QString m_description;
    QUrl m_thumbnailUrl;
    QString m_author;
    QList<Hook> m_hooks;

    QUrl m_url;
    QNetworkReply* m_reply;
    Environment* m_environment;
    bool m_malformed;
};

#endif // EXTENSIONMANIFEST_H
