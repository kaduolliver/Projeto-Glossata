#include "infrastructure/discord/DiscordRichPresence.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QUuid>
#include <QtEndian>

namespace {
constexpr quint32 MaxPayloadSize = 64 * 1024;
}

DiscordRichPresence::DiscordRichPresence(const QString &applicationId, const QString &largeImage,
    QObject *parent, const QStringList &endpoints)
    : QObject(parent), applicationId_(applicationId.trimmed()), largeImage_(largeImage.trimmed().toLower()), endpoints_(endpoints)
{
    if (endpoints_.isEmpty()) {
#ifdef Q_OS_WIN
        // QLocalSocket expands these names to the Windows named-pipe path.
        for (int i = 0; i < 10; ++i) endpoints_ << QStringLiteral("discord-ipc-%1").arg(i);
#else
        QString prefix = qEnvironmentVariable("XDG_RUNTIME_DIR");
        if (prefix.isEmpty()) prefix = qEnvironmentVariable("TMPDIR");
        if (prefix.isEmpty()) prefix = qEnvironmentVariable("TMP");
        if (prefix.isEmpty()) prefix = qEnvironmentVariable("TEMP");
        if (prefix.isEmpty()) prefix = QStringLiteral("/tmp");
        for (int i = 0; i < 10; ++i) endpoints_ << prefix + QStringLiteral("/discord-ipc-%1").arg(i);
#endif
    }
    retryTimer_.setSingleShot(true);
    retryTimer_.setInterval(15000);
    handshakeTimer_.setSingleShot(true);
    handshakeTimer_.setInterval(5000);
    connect(&retryTimer_, &QTimer::timeout, this, [this] { endpointIndex_ = 0; connectNextEndpoint(); });
    connect(&handshakeTimer_, &QTimer::timeout, this, [this] {
        socket_.abort();
        connectionLost();
    });
    connect(&socket_, &QLocalSocket::connected, this, [this] {
        setStatus(QStringLiteral("Conectando ao Discord"));
        handshakeTimer_.start();
        sendFrame(0, QJsonDocument(QJsonObject{{"v", 1}, {"client_id", applicationId_}}).toJson(QJsonDocument::Compact));
    });
    connect(&socket_, &QLocalSocket::readyRead, this, &DiscordRichPresence::readFrames);
    connect(&socket_, &QLocalSocket::disconnected, this, &DiscordRichPresence::connectionLost);
    connect(&socket_, &QLocalSocket::errorOccurred, this, [this](QLocalSocket::LocalSocketError) { connectionLost(); });
}

DiscordRichPresence::~DiscordRichPresence() { stop(); }

void DiscordRichPresence::start()
{
    if (enabled_) return;
    if (!QRegularExpression(QStringLiteral("^[0-9]{17,20}$")).match(applicationId_).hasMatch()) {
        setStatus(QStringLiteral("Application ID do Discord inválido"));
        return;
    }
    enabled_ = true;
    startedAt_ = QDateTime::currentSecsSinceEpoch();
    endpointIndex_ = 0;
    connectNextEndpoint();
}

void DiscordRichPresence::stop()
{
    enabled_ = false;
    retryTimer_.stop();
    handshakeTimer_.stop();
    if (ready_ && socket_.state() == QLocalSocket::ConnectedState) {
        const QJsonObject args{{"pid", QCoreApplication::applicationPid()}, {"activity", QJsonValue::Null}};
        sendFrame(1, QJsonDocument(QJsonObject{{"cmd", "SET_ACTIVITY"}, {"args", args},
            {"nonce", QUuid::createUuid().toString(QUuid::WithoutBraces)}}).toJson(QJsonDocument::Compact));
        socket_.flush();
    }
    ready_ = false;
    socket_.disconnectFromServer();
    incoming_.clear();
    setStatus(QStringLiteral("Desativado"));
}

void DiscordRichPresence::setStatus(const QString &status, bool published)
{
    if (status_ == status && published_ == published) return;
    status_ = status;
    published_ = published;
    emit statusChanged();
}

void DiscordRichPresence::connectNextEndpoint()
{
    if (!enabled_ || socket_.state() != QLocalSocket::UnconnectedState) return;
    incoming_.clear();
    ready_ = false;
    if (endpointIndex_ >= endpoints_.size()) {
        setStatus(QStringLiteral("Aguardando o Discord desktop"));
        retryTimer_.start();
        return;
    }
    const QString endpoint = endpoints_.at(endpointIndex_++);
    socket_.connectToServer(endpoint);
}

void DiscordRichPresence::connectionLost()
{
    handshakeTimer_.stop();
    ready_ = false;
    incoming_.clear();
    if (!enabled_ || retryTimer_.isActive()) return;
    setStatus(QStringLiteral("Aguardando o Discord desktop"));
    // Queue the next attempt to avoid recursion when a pipe does not exist.
    QTimer::singleShot(0, this, &DiscordRichPresence::connectNextEndpoint);
}

void DiscordRichPresence::sendFrame(quint32 opcode, const QByteArray &payload)
{
    QByteArray frame(8, '\0');
    qToLittleEndian<quint32>(opcode, frame.data());
    qToLittleEndian<quint32>(static_cast<quint32>(payload.size()), frame.data() + 4);
    frame.append(payload);
    socket_.write(frame);
}

void DiscordRichPresence::publishActivity()
{
    QJsonObject activity{
        {"type", 0}, {"name", "Glossata Quiz"},
        {"details", QStringLiteral("Explorando os cinco reinos biológicos")},
        {"state", QStringLiteral("Uma descoberta por vez")},
        {"timestamps", QJsonObject{{"start", startedAt_}}}
    };
    if (!largeImage_.isEmpty()) {
        activity.insert("assets", QJsonObject{{"large_image", largeImage_}, {"large_text", "Glossata Quiz"}});
    }
    pendingNonce_ = QUuid::createUuid().toString(QUuid::WithoutBraces);
    const QJsonObject args{{"pid", QCoreApplication::applicationPid()}, {"activity", activity}};
    sendFrame(1, QJsonDocument(QJsonObject{{"cmd", "SET_ACTIVITY"}, {"args", args}, {"nonce", pendingNonce_}}).toJson(QJsonDocument::Compact));
}

void DiscordRichPresence::readFrames()
{
    incoming_.append(socket_.readAll());
    while (incoming_.size() >= 8) {
        const quint32 opcode = qFromLittleEndian<quint32>(incoming_.constData());
        const quint32 length = qFromLittleEndian<quint32>(incoming_.constData() + 4);
        if (length > MaxPayloadSize) { socket_.abort(); return; }
        if (incoming_.size() < 8 + static_cast<qint64>(length)) return;
        const QByteArray payload = incoming_.mid(8, length);
        incoming_.remove(0, 8 + length);
        if (opcode == 3) { sendFrame(4, payload); continue; }
        if (opcode == 2) { socket_.abort(); return; }
        if (opcode != 1) continue;
        const QJsonObject message = QJsonDocument::fromJson(payload).object();
        const QString event = message.value("evt").toString();
        if (event == "READY" && !ready_) {
            handshakeTimer_.stop();
            ready_ = true;
            publishActivity();
        } else if (event == "ERROR") {
            setStatus(QStringLiteral("Discord: %1").arg(message.value("data").toObject().value("message").toString()));
        } else if (message.value("cmd").toString() == "SET_ACTIVITY"
            && message.value("nonce").toString() == pendingNonce_) {
            setStatus(QStringLiteral("Atividade publicada no Discord"), true);
        }
    }
}
