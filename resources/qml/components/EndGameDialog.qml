pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Dialog {
    id: dialog
    objectName: "endGameDialog"
    modal: true; focus: true
    width: Math.min(460,parent ? parent.width-32 : 460)
    anchors.centerIn: parent
    padding: 24
    closePolicy: Popup.CloseOnEscape
    header: Label { text: "Encerrar partida?"; color: dialog.palette.text; font.pixelSize: 23; font.weight: Font.DemiBold; padding: 24 }
    background: Rectangle { radius: 18; color: dialog.palette.window; border.color: dialog.palette.mid }
    contentItem: ColumnLayout {
        spacing: 22
        Label { text: "Tem certeza que deseja encerrar a partida? Sua pontuação atual será registrada no ranking."; color: dialog.palette.text; font.pixelSize: 14; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        UiButton { id: keepPlaying; objectName: "continueGameButton"; text: "Quero continuar jogando"; iconName: "play"; Layout.fillWidth: true; onClicked: dialog.reject(); Keys.onReturnPressed: dialog.reject(); Keys.onEnterPressed: dialog.reject() }
        UiButton { objectName: "endGameButton"; text: "Encerrar partida"; secondary: true; Layout.fillWidth: true; onClicked: dialog.accept(); Keys.onReturnPressed: dialog.accept(); Keys.onEnterPressed: dialog.accept() }
    }
    onOpened: keepPlaying.forceActiveFocus()
}
