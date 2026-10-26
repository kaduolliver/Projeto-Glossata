pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"
import "../style" as Style

Item {
    id: view
    required property var app
    required property var backend
    objectName: "settingsView"
    implicitHeight: settingsLayout.implicitHeight
    property string renameFeedback: ""

    function rename() {
        if (backend.renameCurrentPlayerProfile(profileName.text)) profileName.text = backend.currentPlayerName
        renameFeedback = backend.feedbackText
    }
    Connections {
        target: view.backend
        function onCurrentPlayerNameChanged() { profileName.text = view.backend.currentPlayerName }
    }
    component Section: ColumnLayout {
        id: section
        required property string title
        required property string iconName
        Layout.fillWidth: true
        spacing: Style.Theme.spaceMd
        RowLayout {
            spacing: Style.Theme.spaceSm
            UiIcon { name: section.iconName; ink: Style.Theme.green; implicitWidth: 20; implicitHeight: 20 }
            Label { text: section.title; color: Style.Theme.text; font.pixelSize: Style.Theme.subtitleSize; font.weight: Font.DemiBold; Accessible.role: Accessible.Heading }
        }
    }
    component Divider: Rectangle {
        Layout.fillWidth: true
        implicitHeight: 1
        color: Style.Theme.border
        opacity: 0.55
    }
    component PreferenceSwitch: Switch {
        id: toggle
        implicitWidth: 52
        implicitHeight: Style.Theme.compactHeight
        activeFocusOnTab: true
        hoverEnabled: true
        indicator: Rectangle {
            x: 4; y: (toggle.height - height) / 2
            width: 44; height: 24; radius: 12
            color: toggle.checked ? Style.Theme.primary : toggle.hovered ? Style.Theme.surfaceHover : Style.Theme.surfaceRaised
            border.color: toggle.visualFocus ? Style.Theme.focus : Style.Theme.border
            border.width: toggle.visualFocus ? Style.Theme.focusWidth : 1
            Rectangle { x: toggle.checked ? 24 : 4; y: 4; width: 16; height: 16; radius: 8; color: Style.Theme.text }
        }
        contentItem: Item {}
    }
    component Description: Label {
        Layout.fillWidth: true
        color: Style.Theme.textMuted
        font.pixelSize: Style.Theme.captionSize
        wrapMode: Text.WordWrap
        textFormat: Text.PlainText
    }
    ColumnLayout {
        id: settingsLayout
        width: Math.min(view.width, 1000)
        x: Math.round((view.width - width) / 2)
        spacing: Style.Theme.spaceLg
        Description { text: "Seu perfil e as preferências deste computador. As alterações são salvas localmente." }
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: sections.implicitHeight + 40
            radius: Style.Theme.cardRadius
            color: Style.Theme.surface
            border.color: Style.Theme.border
            ColumnLayout {
                id: sections
                x: 20; y: 20; width: parent.width - 40
                spacing: Style.Theme.spaceLg
                Section {
                    objectName: "settingsProfileSection"
                    title: "Meu perfil"; iconName: "user"
                    RowLayout {
                        Layout.fillWidth: true
                        Label { objectName: "settingsCurrentName"; text: view.backend.currentPlayerName; textFormat: Text.PlainText; color: Style.Theme.text; font.pixelSize: Style.Theme.bodySize; font.weight: Font.DemiBold; wrapMode: Text.WrapAnywhere; Layout.fillWidth: true }
                        Label { text: "Local · offline"; color: Style.Theme.green; font.pixelSize: Style.Theme.captionSize }
                    }
                    Description { text: "Uma única identidade neste computador, sem login. Alterar o nome preserva resultados, conquistas e histórico." }
                    Label { text: "Nome de exibição"; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.labelSize }
                    RowLayout {
                        Layout.fillWidth: true; spacing: Style.Theme.spaceMd
                        TextField {
                            id: profileName
                            objectName: "profileNameField"
                            Layout.fillWidth: true
                            implicitHeight: Style.Theme.compactHeight
                            text: view.backend.currentPlayerName
                            maximumLength: 32
                            placeholderText: "Nome de exibição"
                            Accessible.name: "Nome de exibição do usuário local"
                            placeholderTextColor: Style.Theme.textDisabled
                            color: Style.Theme.text
                            font.pixelSize: Style.Theme.bodySize
                            padding: Style.Theme.spaceMd
                            selectByMouse: true
                            onAccepted: view.rename()
                            background: Rectangle { radius: Style.Theme.controlRadius; color: Style.Theme.canvas; border.width: profileName.activeFocus ? Style.Theme.focusWidth : 1; border.color: profileName.activeFocus ? Style.Theme.focus : Style.Theme.border }
                        }
                        UiButton { objectName: "renameLocalUserButton"; text: "Alterar nome"; secondary: true; compact: true; onClicked: view.rename() }
                    }
                    Description { objectName: "settingsLocalIdentity"; text: "ID local: " + view.backend.currentPlayerId; wrapMode: Text.WrapAnywhere }
                    Description { visible: view.renameFeedback.length > 0; text: view.renameFeedback; Accessible.role: Accessible.AlertMessage }
                }
                Divider {}
                Section {
                    objectName: "settingsInterfaceSection"
                    title: "Interface e acessibilidade"; iconName: "spark"
                    RowLayout {
                        Layout.fillWidth: true; spacing: Style.Theme.spaceLg
                        ColumnLayout {
                            Layout.fillWidth: true; spacing: Style.Theme.spaceXs
                            Label { text: "Transições suaves"; color: Style.Theme.text; font.pixelSize: Style.Theme.bodySize }
                            Description { text: "Reduza o movimento entre telas e nas notificações de conquistas." }
                        }
                        PreferenceSwitch { objectName: "motionSwitch"; checked: view.app.preferences.motionEnabled; Accessible.name: "Transições suaves"; onToggled: view.app.preferences.motionEnabled = checked }
                    }
                }
                Divider {}
                Section {
                    objectName: "settingsAudioSection"
                    title: "Áudio e notificações"; iconName: "trophy"
                    RowLayout {
                        Layout.fillWidth: true; spacing: Style.Theme.spaceLg
                        ColumnLayout {
                            Layout.fillWidth: true; spacing: Style.Theme.spaceXs
                            Label { text: "Sons de conquistas"; color: Style.Theme.text; font.pixelSize: Style.Theme.bodySize }
                            Description { text: "Um efeito discreto quando uma nova conquista é apresentada." }
                        }
                        PreferenceSwitch { objectName: "achievementSoundsSwitch"; checked: view.app.preferences.achievementSoundsEnabled; Accessible.name: "Sons de conquistas"; onToggled: view.app.preferences.achievementSoundsEnabled = checked }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "Volume dos efeitos"; color: Style.Theme.text; font.pixelSize: Style.Theme.bodySize; Layout.fillWidth: true }
                        Label { objectName: "effectsVolumeLabel"; text: view.app.preferences.effectsVolume + "%"; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.labelSize }
                    }
                    RowLayout {
                        Layout.fillWidth: true; spacing: Style.Theme.spaceLg
                        Slider {
                            id: volume
                            objectName: "effectsVolumeSlider"
                            Layout.fillWidth: true
                            implicitHeight: Style.Theme.compactHeight
                            from: 0; to: 100; stepSize: 1
                            value: view.app.preferences.effectsVolume
                            activeFocusOnTab: true
                            Accessible.name: "Volume dos efeitos"
                            onMoved: view.app.preferences.effectsVolume = Math.round(value)
                            Keys.onPressed: event => {
                                if (event.key === Qt.Key_Home || event.key === Qt.Key_End) {
                                    view.app.preferences.effectsVolume = event.key === Qt.Key_Home ? 0 : 100
                                    event.accepted = true
                                }
                            }
                            background: Rectangle {
                                x: volume.leftPadding; y: (volume.height - height) / 2
                                width: volume.availableWidth; height: 4; radius: 2
                                color: Style.Theme.border
                                Rectangle { width: volume.visualPosition * parent.width; height: parent.height; radius: 2; color: Style.Theme.green }
                            }
                            handle: Rectangle {
                                x: volume.leftPadding + volume.visualPosition * (volume.availableWidth - width)
                                y: (volume.height - height) / 2
                                width: 20; height: 20; radius: 10
                                color: volume.pressed ? Style.Theme.green : Style.Theme.text
                                border.color: volume.visualFocus ? Style.Theme.focus : Style.Theme.border
                                border.width: volume.visualFocus ? Style.Theme.focusWidth : 1
                            }
                        }
                        UiButton { objectName: "previewAchievementSoundButton"; text: "Testar som"; iconName: "play"; secondary: true; compact: true; enabled: view.app.effects.available && view.app.preferences.effectsVolume > 0; onClicked: view.app.effects.preview() }
                    }
                    Description { text: "A prévia usa o volume escolhido, mesmo com os sons de conquistas desativados. Em 0%, o áudio fica silencioso." }
                    Description { objectName: "settingsAudioStatus"; visible: text.length > 0; text: view.app.effects.statusText }
                }
                Divider {}
                Section {
                    objectName: "settingsIntegrationsSection"
                    title: "Integrações"; iconName: "butterfly"
                    RowLayout {
                        Layout.fillWidth: true; spacing: Style.Theme.spaceLg
                        ColumnLayout {
                            Layout.fillWidth: true; spacing: Style.Theme.spaceXs
                            Label { text: "Compartilhar atividade no Discord"; color: Style.Theme.text; font.pixelSize: Style.Theme.bodySize; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                            Description { text: view.app.presence.status + ". Esta opção vale para a sessão atual." }
                        }
                        PreferenceSwitch { objectName: "discordPresenceSwitch"; checked: view.app.presence.enabled; Accessible.name: "Compartilhar atividade no Discord"; onToggled: view.app.presence.enabled = checked }
                    }
                }
                Divider {}
                Section {
                    objectName: "settingsInformationSection"
                    title: "Informações"; iconName: "book"
                    Description { text: "Consulte os créditos, as fontes, a transparência e o licenciamento do projeto." }
                    TextAction { objectName: "creditsButton"; text: "Créditos, fontes e transparência"; onClicked: view.app.navigate("credits") }
                }
            }
        }
        ColumnLayout {
            Layout.fillWidth: true
            visible: view.app.preferences.persistenceError.length > 0
            Description { objectName: "settingsPersistenceError"; text: view.app.preferences.persistenceError; Accessible.role: Accessible.AlertMessage }
            UiButton { objectName: "retryPreferencesSaveButton"; text: "Tentar salvar preferências"; visible: view.app.preferences.canRetryPersistence; secondary: true; compact: true; onClicked: view.app.preferences.retrySave() }
        }
    }
}
