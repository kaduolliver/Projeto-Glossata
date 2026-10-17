pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

Item {
    id: view
    required property var app
    required property var backend
    readonly property var details: backend.answerDetails
    readonly property var profile: details.profile || ({})
    property bool showClassification: false
    property bool showSources: false
    function reviewText(note) {
        const review = (note || ({})).governance || ({})
        const evidence = review.evidence || []
        const expired = !!review.due_at && Date.parse(review.due_at) <= Date.now()
        return (expired ? "Revisão vencida" : (review.label || "Revisão humana não documentada"))
            + " · Responsável declarado: " + (review.reviewer || "não registrado")
            + " · Autoria não autenticada por estes registros e hashes"
            + " · Data: " + (review.reviewed_at || "não comprovada")
            + " · Escopo: " + (review.claim_scope || "a validar")
            + (review.due_at ? " · Prazo: " + review.due_at : "")
            + "\n" + (evidence.length ? evidence.map(item => item.locator + " — " + item.supports).join("; ") : "Localização da evidência e escopo da afirmação ainda precisam de revisão.")
    }
    readonly property var sections: [
        {key: "distribution", title: "Distribuição e habitat", note: profile.distribution || ({}), icon: "pin", color: "#67ABEE"},
        {key: "nutrition", title: details.kingdom === "Animalia" ? "Alimentação" : "Nutrição", note: profile.nutrition || ({}), icon: "food", color: "#72D585"},
        {key: "morphology", title: "Morfologia · aspectos físicos", note: profile.morphology || ({}), icon: "shape", color: "#9EBBEF"},
        {key: "conservation", title: "Conservação", note: profile.conservation || ({}), icon: "shield", color: "#E0B75C"},
        {key: "curiosity", title: "Você sabia?" + (details.curiosityScope ? " · " + details.curiosityScope : ""), note: profile.curiosity || {text: details.fact}, icon: "spark", color: "#E0B75C"},
        {key: "hint", title: "Uma pista para reconhecer", note: profile.hint || {text: details.hint}, icon: "leaf", color: "#A4CE9E"}
    ]
    readonly property var references: {
        const result = []
        function add(field, note) {
            if (!note || !note.source_url) return
            const existing = result.find(item => item.url === note.source_url)
            if (existing) existing.fields += " · " + field
            else result.push({title: note.source_title || field, fields: field, url: note.source_url})
        }
        add("Apresentação", profile.summary)
        for (const section of sections) add(section.title, section.note)
        add("Curiosidade", {source_url: details.curiositySource, source_title: details.curiositySourceTitle})
        add("Dica", {source_url: details.hintSource, source_title: details.hintSourceTitle})
        add("Classificação", {source_url: details.sourceUrl, source_title: "iNaturalist · classificação"})
        add("Conservação", {source_url: (profile.conservation || ({})).assessment_url, source_title: "IUCN · avaliação citada"})
        return result
    }
    implicitHeight: Math.max(580, infoLayout.implicitHeight)
    ColumnLayout {
        id: infoLayout
        anchors.fill: parent
        spacing: 24
        RowLayout {
            UiButton { text: "Voltar"; iconName: "back"; secondary: true; compact: true; onClicked: view.app.navigate("feedback") }
            Label { text: "Conheça a espécie"; color: "#EEF7F0"; font.pixelSize: 26; font.weight: Font.DemiBold; Layout.leftMargin: 14 }
        }
        RowLayout {
            Layout.fillWidth: true; spacing: 32; Layout.alignment: Qt.AlignTop
            SpeciesPortrait { Layout.preferredWidth: view.width * 0.36; Layout.preferredHeight: view.width < 850 ? 450 : 520; Layout.minimumHeight: 350; Layout.maximumHeight: 520; Layout.alignment: Qt.AlignTop; info: view.details }
            ColumnLayout {
                Layout.fillWidth: true; spacing: 18
                Label {
                    objectName: "speciesDisplayName"
                    text: view.profile.displayName || view.details.speciesName || ""
                    color: "#F2F8EE"; font.pixelSize: 28; font.weight: Font.DemiBold
                    textFormat: Text.PlainText; wrapMode: Text.WordWrap; Layout.fillWidth: true
                }
                Label { text: view.details.speciesName || ""; color: "#B6CBBF"; font.pixelSize: 18; font.italic: true; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                Label {
                    objectName: "speciesSummary"
                    visible: !!text; text: (view.profile.summary || ({})).text || ""
                    color: "#B6CBBF"; font.pixelSize: 14; lineHeight: 1.3; textFormat: Text.PlainText; wrapMode: Text.WordWrap; Layout.fillWidth: true
                }
                Label { text: "Domínio: " + (view.details.domain || "Não informado") + " · Linhagem: " + (view.details.kingdom || "Não informada"); color: "#86A5AA"; font.pixelSize: 13; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                Label { text: "Conteúdo legado em revisão. As fontes citadas e os testes automáticos não equivalem a validação por especialista. Uma ocorrência fotografada não demonstra toda a distribuição da espécie."; color: "#D9BA7D"; font.pixelSize: 12; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                Label { objectName: "speciesReviewStatus"; visible: view.showSources; text: view.reviewText(view.profile.summary); color: "#86A5AA"; font.pixelSize: 12; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                Repeater {
                    model: view.sections
                    delegate: RowLayout {
                        id: detail
                        required property var modelData
                        objectName: "speciesSection_" + modelData.key
                        visible: !!modelData.note.text
                        Layout.fillWidth: true; spacing: 16
                        Rectangle { implicitWidth: 38; implicitHeight: 38; radius: 12; color: "#172E36"; Layout.alignment: Qt.AlignTop; UiIcon { anchors.centerIn: parent; name: detail.modelData.icon; ink: detail.modelData.color; implicitWidth: 22; implicitHeight: 22 } }
                        ColumnLayout {
                            Layout.fillWidth: true; spacing: 5
                            Label { text: detail.modelData.title; color: "#EDF4EE"; font.pixelSize: 14; font.weight: Font.DemiBold; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                            Label { text: detail.modelData.note.text || ""; color: "#91ADB2"; font.pixelSize: 14; lineHeight: 1.3; textFormat: Text.PlainText; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                            Label { visible: view.showSources; text: view.reviewText(detail.modelData.note); color: "#D0B986"; font.pixelSize: 11; textFormat: Text.PlainText; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                            TextAction { visible: view.showSources && !!detail.modelData.note.source_url; text: (detail.modelData.note.source_title || "Fonte deste tópico") + " ↗"; onClicked: Qt.openUrlExternally(detail.modelData.note.source_url) }
                            Repeater {
                                model: view.showSources ? ((detail.modelData.note.governance || ({})).evidence || []) : []
                                delegate: TextAction {
                                    required property var modelData
                                    text: "Evidência: " + modelData.locator + " ↗"
                                    onClicked: Qt.openUrlExternally(modelData.url)
                                }
                            }
                        }
                    }
                }
                TextAction { objectName: "speciesClassificationButton"; text: (view.showClassification ? "Ocultar" : "Ver") + " classificação científica"; onClicked: view.showClassification = !view.showClassification }
                Label { visible: view.showClassification; text: view.details.classification || "Não informada"; color: "#91ADB2"; font.pixelSize: 13; lineHeight: 1.3; textFormat: Text.PlainText; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                Label { visible: view.showClassification; text: "Classificação de origem: " + (view.profile.taxonomySystem || "iNaturalist") + "\n" + view.reviewText({governance: (view.profile.governance || ({})).taxonomy || ({})}) + "\nEnquadramento filogenético: " + view.reviewText({governance: (view.profile.governance || ({})).phylogeny || ({})}) + "\n" + ((view.profile.phylogeny || ({})).limitations || "") + "\n" + ((view.profile.phylogeny || ({})).system_limitations || ""); color: "#86A5AA"; font.pixelSize: 12; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                Repeater {
                    model: view.showClassification ? ((view.profile.phylogeny || ({})).sources || []) : []
                    delegate: TextAction {
                        required property var modelData
                        Layout.fillWidth: true
                        text: modelData.title + " · " + modelData.locator + " ↗"
                        ToolTip.visible: hovered
                        ToolTip.text: modelData.title + " · " + modelData.locator
                        onClicked: Qt.openUrlExternally(modelData.url)
                    }
                }
                TextAction { objectName: "speciesSourcesButton"; visible: view.references.length > 0; text: (view.showSources ? "Ocultar" : "Consultar") + " fontes das informações"; onClicked: view.showSources = !view.showSources }
                ColumnLayout {
                    visible: view.showSources; Layout.fillWidth: true; spacing: 4
                    Label { text: "Datas legadas de consulta não comprovam revisão humana. Fontes externas precisam de internet; textos e créditos permanecem disponíveis offline."; color: "#86A5AA"; font.pixelSize: 12; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                    Repeater {
                        model: view.references
                        delegate: TextAction {
                            required property var modelData
                            Layout.fillWidth: true
                            text: modelData.title + " ↗"
                            ToolTip.visible: hovered
                            ToolTip.text: modelData.fields
                            onClicked: Qt.openUrlExternally(modelData.url)
                        }
                    }
                }
                TextAction {
                    text: "Comunicar erro, atribuição ou pedido de remoção ↗"
                    onClicked: Qt.openUrlExternally((view.profile.issueUrl || "https://github.com/kaduolliver/Projeto-Glossata/issues/new")
                        + "?title=" + encodeURIComponent("Revisão de conteúdo: " + view.details.speciesName)
                        + "&body=" + encodeURIComponent("Tipo: erro científico / atribuição / remoção\nOrganismo: " + view.details.speciesName
                            + "\nID do catálogo: " + view.details.speciesId + "\nID da foto: " + view.details.photoId
                            + "\nTópico e evidência: \nNão inclua dados pessoais nem localidades sensíveis neste relato público."))
                }
                Label { text: "Sem conexão? Anote o nome científico, o ID da foto e o tópico para enviar o relato depois. Pedidos com informações privadas devem seguir o procedimento em docs/REPORTAR_PROBLEMA.md."; color: "#86A5AA"; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                UiButton { objectName: "speciesContinueButton"; text: "Continuar explorando"; iconName: "arrow"; Layout.fillWidth: true; onClicked: view.app.nextQuestion() }
            }
        }
    }
}
