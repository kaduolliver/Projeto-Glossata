pragma ComponentBehavior: Bound
import QtQuick
Item {
    id: composition
    Repeater {
        model: ["Animalia", "Plantae", "Fungi", "Eukaryota", "Bacteria"]
        delegate: KingdomArtwork {
            required property string modelData
            required property int index
            kingdom: modelData
            width: Math.min(composition.width, composition.height) * (index < 3 ? 0.48 : 0.32); height: width
            x: [0.02, 0.46, 0.15, 0.65, 0.48][index] * composition.width
            y: Math.min([0.08, 0.04, 0.5, 0.52, 0.73][index] * composition.height, composition.height - height - 12)
            rotation: [-16, 12, -8, 20, -20][index]
            opacity: index < 3 ? 0.85 : 0.6
        }
    }
}
