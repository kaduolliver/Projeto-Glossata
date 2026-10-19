// SPDX-License-Identifier: Apache-2.0
pragma Singleton
import QtQuick

QtObject {
    // Shared presentation tokens. No gameplay or content policy lives here.
    property bool motionEnabled: true
    readonly property int durationFast: motionEnabled ? 120 : 0
    readonly property int durationPage: motionEnabled ? 180 : 0
    readonly property color canvas: "#091A22"
    readonly property color surface: "#10262D"
    readonly property color surfaceRaised: "#18343C"
    readonly property color surfaceHover: "#21444A"
    readonly property color border: "#45646A"
    readonly property color forest: "#174D38"
    readonly property color primary: "#216445"
    readonly property color primaryHover: "#29744E"
    readonly property color primaryPressed: "#164C35"
    readonly property color danger: "#8F3535"
    readonly property color dangerHover: "#A34040"
    readonly property color dangerPressed: "#712B2B"
    readonly property color text: "#F0F5EC"
    readonly property color textMuted: "#B4C9CA"
    readonly property color textDisabled: "#91A6AA"
    readonly property color green: "#85DCAA"
    readonly property color gold: "#E2C477"
    readonly property color focus: "#F1D58B"
    readonly property color disabledSurface: "#20343A"
    readonly property string fontFamily: "Segoe UI"
    readonly property int titleSize: 30
    readonly property int subtitleSize: 20
    readonly property int bodySize: 14
    readonly property int labelSize: 13
    readonly property int captionSize: 12
    readonly property int metricSize: 28
    readonly property font scientificFont: Qt.font({family: fontFamily, pixelSize: bodySize, italic: true})
    readonly property int spaceXs: 6
    readonly property int spaceSm: 10
    readonly property int spaceMd: 12
    readonly property int spaceLg: 20
    readonly property int navPadding: 14
    readonly property int controlRadius: 10
    readonly property int cardRadius: 16
    readonly property int borderWidth: 1
    readonly property int focusWidth: 2
    readonly property int controlHeight: 50
    readonly property int compactHeight: 40
    readonly property int navHeight: 46
    readonly property int actionHeight: 36
}
