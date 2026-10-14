pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout {
    id: portrait
    property var info: ({})
    readonly property bool hasPhoto: !!info.image && photo.status === Image.Ready
    spacing: 10
    Rectangle {
        Layout.fillWidth: true; Layout.fillHeight: true; Layout.minimumHeight: 240
        radius: 18; color: "#112F2D"; border.color: "#304D43"; clip: true
        Image { id: photo; anchors.fill: parent; anchors.margins: 8; source: portrait.info.image || ""; fillMode: Image.PreserveAspectFit; smooth: true; mipmap: true }
        KingdomArtwork { anchors.fill: parent; anchors.margins: 20; visible: !portrait.hasPhoto; kingdom: portrait.info.kingdom || "Animalia" }
        Label { visible: !portrait.hasPhoto; anchors.bottom: parent.bottom; anchors.horizontalCenter: parent.horizontalCenter; anchors.bottomMargin: 15; text: portrait.info.image ? "Imagem indisponível" : "Ilustração decorativa"; color: "#86A59A"; font.pixelSize: 10 }
    }
    Label { visible: portrait.hasPhoto; Layout.fillWidth: true; text: portrait.info.author || ""; textFormat: Text.PlainText; color: "#8FA9AA"; font.pixelSize: 10; wrapMode: Text.WordWrap }
    RowLayout {
        visible: portrait.hasPhoto; Layout.fillWidth: true
        Button { text: portrait.info.license === "cc0" ? "CC0" : "CC BY"; flat: true; padding: 0; font.pixelSize: 10; palette.buttonText: "#9DC7AB"; onClicked: Qt.openUrlExternally(portrait.info.licenseUrl || "") }
        Item { Layout.fillWidth: true }
        Button { text: "Créditos e origem ↗"; flat: true; font.pixelSize: 10; onClicked: Qt.openUrlExternally(portrait.info.source || "") }
    }
}
