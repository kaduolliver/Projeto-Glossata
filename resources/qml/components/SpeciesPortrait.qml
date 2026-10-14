pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout {
    id: portrait
    property var info: ({})
    property bool allowDecoration: false
    signal loadFailed()
    readonly property bool hasPhoto: !!info.image && photo.status === Image.Ready
    spacing: 10
    Label { visible: !!portrait.info.supporting; Layout.fillWidth: true; text: "Imagem de apoio · não comprova a identificação da espécie"; color: "#D9BA7D"; font.pixelSize: 11; wrapMode: Text.WordWrap }
    Rectangle {
        Layout.fillWidth: true; Layout.fillHeight: true; Layout.minimumHeight: 240
        radius: 18; color: "#112F2D"; border.color: "#304D43"; clip: true
        Image { id: photo; anchors.fill: parent; anchors.margins: 8; source: portrait.info.image || ""; fillMode: Image.PreserveAspectFit; smooth: true; mipmap: true; onStatusChanged: if (status === Image.Error) portrait.loadFailed() }
        KingdomArtwork { anchors.fill: parent; anchors.margins: 20; visible: portrait.allowDecoration && !portrait.hasPhoto; kingdom: portrait.info.kingdom || "Animalia" }
        Label { visible: !portrait.hasPhoto; anchors.centerIn: parent; width: parent.width-28; horizontalAlignment: Text.AlignHCenter; wrapMode: Text.WordWrap; text: portrait.allowDecoration ? "Ilustração decorativa" : (photo.status === Image.Loading ? "Carregando fotografia..." : "Fotografia indisponível. Esta questão não deve ser respondida."); color: "#86A59A"; font.pixelSize: 12 }
    }
    Label { visible: portrait.hasPhoto; Layout.fillWidth: true; text: portrait.info.author || ""; textFormat: Text.PlainText; color: "#8FA9AA"; font.pixelSize: 10; wrapMode: Text.WordWrap }
    RowLayout {
        visible: portrait.hasPhoto; Layout.fillWidth: true
        TextAction { text: portrait.info.license === "cc0" ? "CC0" : "CC BY"; font.pixelSize: 10; onClicked: Qt.openUrlExternally(portrait.info.licenseUrl || "") }
        Item { Layout.fillWidth: true }
        TextAction { text: "Créditos e origem ↗"; font.pixelSize: 10; onClicked: Qt.openUrlExternally(portrait.info.source || "") }
    }
}
