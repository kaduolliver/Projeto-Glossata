pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
ComboBox {
    id: control
    property string emptyText: "Sem nível visual disponível"
    implicitHeight: 46
    padding: 12; leftPadding: 16; rightPadding: 42
    hoverEnabled: true
    font.pixelSize: 13
    opacity: enabled ? 1 : 0.45
    background: Rectangle {
        objectName: "taxonComboBackground"
        radius: 12
        color: control.down ? control.palette.highlight : (control.hovered ? Qt.lighter(control.palette.button,1.12) : control.palette.button)
        border.color: control.activeFocus ? control.palette.light : control.palette.mid
        border.width: control.activeFocus ? 2 : 1
    }
    contentItem: Text { text: control.count ? control.displayText : control.emptyText; font: control.font; color: control.palette.text; verticalAlignment: Text.AlignVCenter; elide: Text.ElideRight }
    indicator: UiIcon { name: "arrow"; rotation: 90; ink: control.palette.text; width: 16; height: 16; x: control.width-width-16; y: (control.height-height)/2 }
    delegate: ItemDelegate {
        id: option
        required property int index
        width: ListView.view ? ListView.view.width : control.width
        implicitHeight: 42
        text: control.textAt(index)
        highlighted: control.highlightedIndex === index
        hoverEnabled: true
        contentItem: Text { text: option.text; font: control.font; color: control.palette.text; verticalAlignment: Text.AlignVCenter; elide: Text.ElideRight }
        background: Rectangle {
            radius: 8; color: option.down ? control.palette.highlight : (option.highlighted || option.hovered ? control.palette.button : "transparent")
            border.color: option.activeFocus ? control.palette.light : "transparent"; border.width: option.activeFocus ? 2 : 0
        }
    }
    popup: Popup {
        objectName: "taxonComboPopup"
        y: control.height+6
        width: control.width
        padding: 8
        implicitHeight: Math.min(list.contentHeight+16,270)
        palette: control.palette
        background: Rectangle { objectName: "taxonPopupBackground"; radius: 12; color: control.palette.window; border.color: control.palette.mid }
        contentItem: ListView {
            id: list
            clip: true
            model: control.delegateModel
            currentIndex: control.highlightedIndex
            highlightMoveDuration: 0
            ScrollIndicator.vertical: ScrollIndicator { }
        }
    }
}
