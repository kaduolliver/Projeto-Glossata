#pragma once

#include <QObject>
#include <QLocalSocket>
#include <QTimer>
#include <QStringList>

// Minimal local Discord RPC integration: no bot token or OAuth credentials.
class DiscordRichPresence final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY statusChanged)
public:
    explicit DiscordRichPresence(const QString &applicationId, const QString &largeImage,
        QObject *parent = nullptr, const QStringList &endpoints = {});
    ~DiscordRichPresence() override;
    void start();
    void stop();
    QString status() const { return status_; }
    bool activityPublished() const { return published_; }
    bool enabled() const { return enabled_; }
    void setEnabled(bool enabled) { enabled ? start() : stop(); }

signals:
    void statusChanged();

private:
    void connectNextEndpoint();
    void connectionLost();
    void readFrames();
    void publishActivity();
    void sendFrame(quint32 opcode, const QByteArray &payload);
    void setStatus(const QString &status, bool published = false);

    QString applicationId_;
    QString largeImage_;
    QStringList endpoints_;
    QLocalSocket socket_;
    QTimer retryTimer_;
    QTimer handshakeTimer_;
    QByteArray incoming_;
    QString pendingNonce_;
    QString status_ = QStringLiteral("Desativado");
    qint64 startedAt_ = 0;
    int endpointIndex_ = 0;
    bool enabled_ = false;
    bool ready_ = false;
    bool published_ = false;
};
