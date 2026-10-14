#include "infrastructure/discord/DiscordRichPresence.h"
#include <QtTest/QtTest>
#include <QLocalServer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>
#include <QtEndian>
#include <memory>

namespace {
QByteArray frame(quint32 opcode, const QByteArray &payload)
{
    QByteArray result(8, '\0');
    qToLittleEndian<quint32>(opcode, result.data());
    qToLittleEndian<quint32>(static_cast<quint32>(payload.size()), result.data() + 4);
    return result + payload;
}
QByteArray jsonFrame(const QJsonObject &object)
{
    return frame(1, QJsonDocument(object).toJson(QJsonDocument::Compact));
}
bool frameAvailable(QLocalSocket *socket, QByteArray &buffer)
{
    buffer += socket->readAll();
    return buffer.size() >= 8 && buffer.size() >= 8 + qFromLittleEndian<quint32>(buffer.constData() + 4);
}
QPair<quint32, QByteArray> takeFrame(QByteArray &buffer)
{
    const quint32 opcode = qFromLittleEndian<quint32>(buffer.constData());
    const quint32 size = qFromLittleEndian<quint32>(buffer.constData() + 4);
    const QByteArray payload = buffer.mid(8, size);
    buffer.remove(0, 8 + size);
    return {opcode, payload};
}
QString uniqueEndpoint()
{
    return "glossata-test-" + QUuid::createUuid().toString(QUuid::WithoutBraces);
}
}

class DiscordRichPresenceTests : public QObject {
    Q_OBJECT
private slots:
    void publishesAssetHandlesPingAndClearsActivity();
    void missingDiscordDoesNotBlockEventLoop();
    void invalidApplicationIdDoesNotConnect();
};

void DiscordRichPresenceTests::publishesAssetHandlesPingAndClearsActivity()
{
    QLocalServer server;
    const QString endpoint = uniqueEndpoint();
    QVERIFY(server.listen(endpoint));
    DiscordRichPresence presence("1559712085250801685", "GLOSSATA_ICON", nullptr, {endpoint});
    presence.start();
    QTRY_VERIFY(server.hasPendingConnections());
    std::unique_ptr<QLocalSocket> peer(server.nextPendingConnection());
    QByteArray buffer;
    QTRY_VERIFY(frameAvailable(peer.get(), buffer));
    const auto handshake = takeFrame(buffer);
    QCOMPARE(handshake.first, quint32(0));
    const auto handshakeJson = QJsonDocument::fromJson(handshake.second).object();
    QCOMPARE(handshakeJson.value("v").toInt(), 1);
    QCOMPARE(handshakeJson.value("client_id").toString(), QString("1559712085250801685"));

    const QByteArray ready = jsonFrame(QJsonObject{{"cmd", "DISPATCH"}, {"evt", "READY"}});
    peer->write(ready.left(3));
    peer->flush();
    QTest::qWait(15);
    QVERIFY(!presence.activityPublished());
    peer->write(ready.mid(3));
    QTRY_VERIFY(frameAvailable(peer.get(), buffer));
    const auto activityFrame = takeFrame(buffer);
    QCOMPARE(activityFrame.first, quint32(1));
    const QJsonObject command = QJsonDocument::fromJson(activityFrame.second).object();
    QCOMPARE(command.value("cmd").toString(), QString("SET_ACTIVITY"));
    const QJsonObject activity = command.value("args").toObject().value("activity").toObject();
    QCOMPARE(activity.value("assets").toObject().value("large_image").toString(), QString("glossata_icon"));
    QCOMPARE(activity.value("name").toString(), QString("Glossata Quiz"));
    QVERIFY(!activity.contains("playerName"));
    QVERIFY(!presence.activityPublished());
    peer->write(jsonFrame({{"cmd", "SET_ACTIVITY"}, {"nonce", command.value("nonce")}, {"data", activity}}));
    QTRY_VERIFY(presence.activityPublished());

    peer->write(frame(3, "ping-test"));
    QTRY_VERIFY(frameAvailable(peer.get(), buffer));
    const auto pong = takeFrame(buffer);
    QCOMPARE(pong.first, quint32(4));
    QCOMPARE(pong.second, QByteArray("ping-test"));

    peer->write(jsonFrame({{"cmd", "SET_ACTIVITY"}, {"evt", "ERROR"},
        {"nonce", command.value("nonce")}, {"data", QJsonObject{{"message", "Asset ausente"}}}}));
    QTRY_VERIFY(!presence.activityPublished());
    QVERIFY(presence.status().contains("Asset ausente"));

    presence.stop();
    QVERIFY(!presence.enabled());
    QTRY_VERIFY(frameAvailable(peer.get(), buffer));
    const QJsonObject cleared = QJsonDocument::fromJson(takeFrame(buffer).second).object();
    QCOMPARE(cleared.value("cmd").toString(), QString("SET_ACTIVITY"));
    QVERIFY(cleared.value("args").toObject().value("activity").isNull());
}

void DiscordRichPresenceTests::missingDiscordDoesNotBlockEventLoop()
{
    DiscordRichPresence presence("1559712085250801685", "glossata_icon", nullptr, {uniqueEndpoint()});
    bool eventDelivered = false;
    QTimer::singleShot(10, [&] { eventDelivered = true; });
    presence.start();
    QTRY_VERIFY(eventDelivered);
    QTRY_VERIFY(presence.status().contains("Aguardando"));
    QVERIFY(!presence.activityPublished());
    presence.stop();
}

void DiscordRichPresenceTests::invalidApplicationIdDoesNotConnect()
{
    QLocalServer server;
    const QString endpoint = uniqueEndpoint();
    QVERIFY(server.listen(endpoint));
    DiscordRichPresence presence("not-an-id", "glossata_icon", nullptr, {endpoint});
    presence.start();
    QVERIFY(!presence.enabled());
    QVERIFY(presence.status().contains("inválido"));
    QVERIFY(!server.hasPendingConnections());
}

QTEST_GUILESS_MAIN(DiscordRichPresenceTests)
#include "DiscordRichPresenceTests.moc"
