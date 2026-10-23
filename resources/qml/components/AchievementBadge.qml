pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Shapes
import "AchievementBadgePaths.js" as BadgePaths

Item {
    id: badge
    property string glyph: "leaf"
    readonly property var drawing: BadgePaths.drawings[glyph] || BadgePaths.drawings.leaf
    implicitWidth: 96
    implicitHeight: 106
    Item {
        width: 120; height: 132
        x: (badge.width - width * scale) / 2; y: (badge.height - height * scale) / 2
        scale: Math.min(badge.width / width, badge.height / height)
        transformOrigin: Item.TopLeft
        Shape {
            anchors.fill: parent
            preferredRendererType: Shape.CurveRenderer
            ShapePath { strokeColor: badge.drawing.color; strokeWidth: 1.5; fillColor: "#183c32"; PathSvg { path: "M28 91L21 127L39 118L49 129L56 98M64 98L71 129L81 118L99 127L92 91" } }
            ShapePath {
                strokeWidth: 0
                fillGradient: LinearGradient {
                    x1: 10; y1: 4; x2: 110; y2: 110
                    GradientStop { position: 0; color: "#fff1bd" }
                    GradientStop { position: 0.4; color: badge.drawing.color }
                    GradientStop { position: 1; color: "#79592c" }
                }
                PathSvg { path: "M60 4L105 30L109 80L60 110L11 80L15 30Z" }
            }
            ShapePath { strokeColor: "#f5dda0"; strokeWidth: 1; fillColor: "#102b2c"; PathSvg { path: "M60 10L100 33L103 77L60 103L17 77L20 33Z" } }
            ShapePath { strokeColor: badge.drawing.color; strokeWidth: 0.5; fillColor: "transparent"; PathSvg { path: "M60 16L94 36L97 74L60 96L23 74L26 36Z" } }
            ShapePath { strokeWidth: 0; fillColor: "#fff0b9"; PathSvg { path: "M60 8L63 13L60 18L57 13Z" } }
        }
        Shape {
            x: 17; y: 15; width: 100; height: 100; scale: 0.86; transformOrigin: Item.TopLeft
            preferredRendererType: Shape.CurveRenderer
            ShapePath {
                strokeColor: badge.drawing.color; strokeWidth: 3; fillColor: "transparent"
                capStyle: ShapePath.RoundCap; joinStyle: ShapePath.RoundJoin
                PathSvg { path: badge.drawing.path }
            }
        }
    }
}
