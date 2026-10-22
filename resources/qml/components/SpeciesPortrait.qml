pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../style" as Style
ColumnLayout {
    id: portrait
    property var info: ({})
    property bool allowDecoration: false
    property bool quizPresentation: false
    signal loadFailed()
    readonly property bool hasPhoto: !!info.image && photo.status === Image.Ready
    spacing: 10
    Label { visible: !!portrait.info.supporting; Layout.fillWidth: true; text: "Imagem de apoio · não comprova a identificação da espécie"; color: "#D9BA7D"; font.pixelSize: 11; wrapMode: Text.WordWrap }
    Rectangle {
        Layout.fillWidth: true; Layout.fillHeight: true; Layout.minimumHeight: 240
        objectName: "speciesPhotoFrame"
        radius: 18; color: portrait.quizPresentation ? Style.Theme.canvas : "#112F2D"; border.color: portrait.quizPresentation ? Style.Theme.border : "#304D43"; clip: true
        Image {
            id: photo; objectName: "speciesPhotoImage"
            readonly property real fittedScale: sourceSize.width > 0 && sourceSize.height > 0 ? Math.min(1, (parent.width - 16) / sourceSize.width, (parent.height - 16) / sourceSize.height) : 0
            anchors.centerIn: parent
            width: portrait.quizPresentation ? sourceSize.width * fittedScale : parent.width - 16
            height: portrait.quizPresentation ? sourceSize.height * fittedScale : parent.height - 16
            source: portrait.info.image || ""; fillMode: Image.PreserveAspectFit; smooth: true; mipmap: true
            onStatusChanged: if (status === Image.Error) portrait.loadFailed()
        }
        KingdomArtwork { anchors.fill: parent; anchors.margins: 20; visible: portrait.allowDecoration && !portrait.hasPhoto; kingdom: portrait.info.kingdom || "Animalia" }
        Label { visible: !portrait.hasPhoto; anchors.centerIn: parent; width: parent.width-28; horizontalAlignment: Text.AlignHCenter; wrapMode: Text.WordWrap; text: portrait.allowDecoration ? "Ilustração decorativa" : (photo.status === Image.Loading ? "Carregando fotografia..." : "Fotografia indisponível. Esta questão não deve ser respondida."); color: "#86A59A"; font.pixelSize: 12 }
    }
    Label { visible: portrait.hasPhoto; Layout.fillWidth: true; text: portrait.info.author || ""; textFormat: Text.PlainText; color: "#8FA9AA"; font.pixelSize: 10; wrapMode: Text.WordWrap }
    Label { visible: portrait.hasPhoto && !!text; Layout.fillWidth: true; text: (portrait.info.attribution || ({})).title || ""; textFormat: Text.PlainText; color: "#8FA9AA"; font.pixelSize: 10; wrapMode: Text.WordWrap }
    Label { visible: portrait.hasPhoto; Layout.fillWidth: true; text: portrait.info.modifications || "Alterações não documentadas."; textFormat: Text.PlainText; color: "#8FA9AA"; font.pixelSize: 10; wrapMode: Text.WordWrap }
    Label { visible: ((portrait.info.attribution || ({})).scientific_source_issues || []).length > 0; Layout.fillWidth: true; text: "Registro de origem com pendência de revisão da identificação."; color: "#D9BA7D"; font.pixelSize: 10; wrapMode: Text.WordWrap }
    RowLayout {
        visible: portrait.hasPhoto; Layout.fillWidth: true
        TextAction { text: (portrait.info.license || "Licença não informada").toUpperCase() + ((portrait.info.attribution || ({})).license_version ? " " + portrait.info.attribution.license_version : " · versão não comprovada"); font.pixelSize: 10; onClicked: Qt.openUrlExternally(portrait.info.licenseUrl || (portrait.info.attribution || ({})).license_source_url || "") }
        Item { Layout.fillWidth: true }
        TextAction { text: "Foto e origem ↗"; font.pixelSize: 10; onClicked: Qt.openUrlExternally((portrait.info.attribution || ({})).photo_url || portrait.info.source || "") }
    }
}
