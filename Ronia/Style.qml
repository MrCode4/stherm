pragma Singleton

import QtQuick

import Ronia.impl

QtObject {
    id: _style

    enum Theme {
        Light,
        Dark
    }

    property int        theme:                          Style.Theme.Light
    property color      background:                     "#040404"
    property color      foreground:                     "#ffffff"
    property color      accent:                         "#9B0EC2"
    property color      primary:                        "#E8EAF6"
    property color      frameColor:                     "#f1f1f1"
    property color      backgroundDimColor:             "#F0363636"
    property color      listHighlightColor:             "#1E626262"
    property color      disabledColor:                  "#404040"
    property color      dropShadowColor:                "#0F000000"
    property color      hintTextColor:                  "#60FFFFFF"
    property color      hoverColor:                     "#7F7F7F"
    property color      rippleColor:                    Qt.alpha(foreground, 0.16)
    property color      highlightedRippleColor:         Qt.alpha(accent, 0.24)

    //! Specific styles for some Controls
    readonly property FontIconsSize fontIconSize: FontIconsSize {
        largePt: Application.font.pointSize * 1.6
        normalPt: Application.font.pointSize * 1.3
        smallPt: Application.font.pointSize * 1.
    }

    //! Button
    readonly property ButtonStyle button: ButtonStyle {
        background: _style.background
        hoverColor: "#606060"
        disabledColor: "#242424"
        buttonHeight: 32
        radius: -1 //! i.e Full radius (Math.min(height, width) / 2
        horizontalPadding: 20
        verticalPadding: horizontalPadding - 12
        borderWidth: 1
    }
}
