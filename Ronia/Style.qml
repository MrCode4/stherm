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
    property color      foreground:                     "#FFFFFF"
    property color      accent:                         "#FFFFFF"
    property color      primary:                        "#E8EAF6"

    property color      secondaryTextColor:             "#BBBBBB"
    property color      frameColor:                     "#F1F1F1"
    property color      backgroundDimColor:             "#F0363636"
    property color      listHighlightColor:             "#1E626262"
    property color      disabledColor:                  "#404040"
    property color      dropShadowColor:                "#0F000000"
    property color      hintTextColor:                  "#60FFFFFF"
    property color      hoverColor:                     "#7F7F7F"
    property color      rippleColor:                    Qt.alpha(foreground, 0.16)
    property color      highlightedRippleColor:         Qt.alpha(accent, 0.24)

    //! App specific shades of main colors
    property color      green:      "#4EAC55"

    //! Number properties
    property int        delegateHeight:     64
    property int        textFieldHeight:    72
    property int        touchTarget:        56

    //! Specific styles for some Controls
    readonly property FontIconsSize fontIconSize: FontIconsSize {
        largePt: Application.font.pointSize * 1.8
        normalPt: Application.font.pointSize * 1.3
        smallPt: Application.font.pointSize * 1.
    }

    //! Button
    readonly property ButtonStyle button: ButtonStyle {
        background: _style.background
        hoverColor: "#606060"
        disabledColor: "#242424"
        buttonHeight: 48
        radius: -1 //! i.e Full radius (Math.min(height, width) / 2
        horizontalPadding: 20
        verticalPadding: horizontalPadding - 10
        borderWidth: 1
    }

    //! RadioButton
    readonly property RadioButtonStyle radioButton: RadioButtonStyle {
        indicatorSize: 28
        indicatorInnerCircleSize: 14
    }

    //! Switch
    readonly property SwitchStyle switchStyle: SwitchStyle {
        switchCheckedTrackColor: _style.green
    }
}
