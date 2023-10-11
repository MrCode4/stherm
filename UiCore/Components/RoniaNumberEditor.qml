import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Universal
import QTemplateProject

/*! ***********************************************************************************************
 * RoniaNumberEditor allows users to enter integer and float numbers direcly, or by draging the mouse
 * to the left/right to decrease/increase the value.
 * ************************************************************************************************/
TextField {
    id : root

    /* Object Properties
     * ****************************************************************************************/
    width: 60
    height: 30
    clip: true


    verticalAlignment: TextInput.AlignVCenter
    horizontalAlignment: TextInput.AlignHCenter
    font.pixelSize: root.fontSize
    padding: 5
    font.bold: true
    wrapMode: Text.WordWrap
    selectByMouse: !mouseDragEdit

    /* Property Declarations
     * ****************************************************************************************/
    property alias  value: root.text
    property real   from: 0             // default should be -infinity?
    property real   to: 10000           // default should be inifity?
    property real   stepSize: 1
    property int    initialValue
    property bool   doubleStep: false
    property int    decimals: 0
    property bool   numeric: true
    property int    fontSize: 13
    property bool   mouseDragEdit: false
    property color  backgroundColor: "transparent"
    property int    radius: 0
    property bool   focusable: !root.readOnly
    property bool   progressiveRect: false

    signal maxValueExceeded();

    Keys.onReturnPressed: {
        // Deselect the TextField
        root.cursorPosition = 0 // Move cursor to the beginning of the text to deselect
        root.focus = false;
    }

    /* Children: TextField (editor)
     * ****************************************************************************************/

    // we change back color to transparent when the editor is not focused
    background: Rectangle {
        border.width: 2
        color: root.backgroundColor
        radius: root.radius
        border.color: (focusable && root.focus) ? "#3E65FF" : "transparent"

        Rectangle {
            height: parent.height
            width: (parseFloat(value) - from) * parent.width / (to - from)
            color: AppStyle.primaryColor
            visible: root.progressiveRect
        }
    }
    // power checking for number
    function checkPowerOfTwo(n, behavior) {
        const minus = n.startsWith("-")
        n = Math.abs(n)
        let power = Math.ceil(Math.log2(n));
        if (Math.pow(2, power) !== n) {
            if (behavior === "inc") {
                n = Math.pow(2, power);
            }
            else {
                n = Math.pow(2, power - 1)
            }
        }
        else{
            if (behavior === "inc") {
                n = Math.pow(2, power + 1)
            }
            else {
                n = Math.pow(2, power - 1)
            }
        }
        return minus ? n * -1 : n
    }

    function calcComboValue(_type){
        let newValue;
        if (parseInt(value) === 0) {
            newValue = _type === "inc" ? parseInt(value) + 1 : parseInt(value) -1
            return newValue
        }
        if (_type === "inc") {
            if (doubleStep)
                newValue = checkPowerOfTwo(value, value.startsWith("-")?  "dec": "inc")
            else
                newValue = parseInt(value) + stepSize
        }
        else{
            if (doubleStep)
                newValue = checkPowerOfTwo(value, value.startsWith("-")?  "inc": "dec")
            else
                newValue = parseInt(value) - stepSize
        }
        return newValue;
    }

    function increment() {
        var newValue = calcComboValue("inc")
        if (newValue > root.to)
            newValue = root.to;
        else if (newValue < root.from)
            newValue = root.from;
        root.clear();

        //! *************** NOTE *************************
        //! Never use assignment operator to update the value.
        //! because it breaks the binding.
        root.insert(0, parseInt(newValue));
    }

    function decrement() {
        var newValue = calcComboValue("dec")
        if (newValue > root.to)
            newValue = root.to;
        else if (newValue < root.from)
            newValue = root.from;

        //! *************** NOTE *************************
        //! Never use assignment operator to update the value.
        //! because it breaks the binding.
        root.clear();
        root.insert(0, parseInt(newValue));
    }
}
