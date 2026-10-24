pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Shapes
import "AchievementBadgePaths.js" as BadgePaths

Item {
    id: badge
    property string glyph: "leaf"
    property string visualLevel: "initial"
    property bool unlocked: false
    readonly property var material: BadgePaths.levels[visualLevel] || BadgePaths.levels.initial
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
            opacity: badge.unlocked ? 1 : 0.65
            preferredRendererType: Shape.CurveRenderer
            ShapePath { strokeColor: badge.material.rim; strokeWidth: 1.5; fillColor: "#183c32"; PathSvg { path: badge.material.ribbon } }
            ShapePath {
                strokeWidth: 0
                fillGradient: LinearGradient {
                    x1: 10; y1: 4; x2: 110; y2: 110
                    GradientStop { position: 0; color: badge.material.light }
                    GradientStop { position: 0.35; color: badge.material.metal }
                    GradientStop { position: 0.55; color: badge.material.light }
                    GradientStop { position: 1; color: badge.material.shade }
                }
                PathSvg { path: badge.material.outer }
            }
            ShapePath { strokeColor: badge.material.rim; strokeWidth: 1; fillColor: "#102b2c"; PathSvg { path: badge.material.inner } }
            ShapePath { strokeColor: badge.material.rim; strokeWidth: 0.7; fillColor: "transparent"; PathSvg { path: badge.material.engraving } }
            ShapePath { strokeColor: badge.material.shade; strokeWidth: 0.4; fillColor: badge.material.light; PathSvg { path: badge.material.ornaments } }
        }
        Shape {
            anchors.fill: parent
            visible: badge.visualLevel === "special"
            opacity: badge.unlocked ? 1 : 0.65
            preferredRendererType: Shape.CurveRenderer
            ShapePath {
                strokeColor: badge.material.shade; strokeWidth: 0.5
                fillGradient: LinearGradient {
                    x1: 8; y1: 30; x2: 110; y2: 100
                    GradientStop { position: 0; color: "#b6d5e6" }
                    GradientStop { position: 0.3; color: "#54758f" }
                    GradientStop { position: 0.65; color: "#abc8dc" }
                    GradientStop { position: 1; color: "#38566f" }
                }
                PathSvg { path: badge.material.facets || "" }
            }
            ShapePath { strokeWidth: 0; fillColor: "#e7f6ff"; PathSvg { path: badge.material.reflections || "" } }
        }
        Shape {
            x: 17; y: 15; width: 100; height: 100; scale: 0.86; transformOrigin: Item.TopLeft
            opacity: badge.unlocked ? 1 : 0.5
            preferredRendererType: Shape.CurveRenderer
            ShapePath {
                strokeColor: badge.drawing.color; strokeWidth: 3; fillColor: "transparent"
                capStyle: ShapePath.RoundCap; joinStyle: ShapePath.RoundJoin
                PathSvg { path: badge.drawing.path }
            }
        }
    }
}
