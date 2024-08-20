import QtQuick
import QtQuick.Controls

/*! ***********************************************************************************************
 * Built-in StackView keeps all the previous pages in memory and so all the bindings in all the
 * previous pages are running (including the timers and objects). The purpose of this SimpleStackView
 * is to replace the built-in StackView as is and solve those problems.
 * ***********************************************************************************************/
Item {
    id: root
    property StackView parentStackView
    readonly property int depth: sv.stack.length

    property alias currentItem: sv.currentItem
    property alias busy: sv.busy
    property alias popExit: sv.popExit
    property alias popEnter: sv.popEnter
    property alias pushExit: sv.pushExit
    property alias pushEnter: sv.pushEnter
    property alias replaceExit: sv.replaceExit
    property alias replaceEnter: sv.replaceEnter

    function updateProps(page, newProps) {
        for(let i = 0; i < sv.stack.length; i++) {
            let vm = sv.stack[i];
            if (vm.page == page) {
                Object.assign(vm.props, newProps);
                break;
            }
        }
    }

    function push(page, props) {
        sv.stack.push({'page': page, 'props': props});
        sv.replaceExit = sv.pushExit;
        sv.replaceEnter = sv.pushEnter;
        sv.replace(sv.currentItem, page, props);
    }

    function pop() {
        if (sv.stack.length > 1) {
            sv.stack.splice(-1, 1);
            let vm = sv.stack[sv.stack.length-1];
            sv.replaceExit = sv.popExit;
            sv.replaceEnter = sv.popEnter;
            sv.replace(sv.currentItem, vm.page, vm.props)
        }
        else {
            if (parentStackView) {
                parentStackView.pop();
            }
        }
    }

    StackView {
        id: sv
        property var stack: []
        anchors.fill: parent
        initialItem: Item {}

        property Transition oldReplaceExit
        property Transition oldReplaceEnter

        Component.onCompleted: {
            oldReplaceExit = replaceExit
            oldReplaceEnter = replaceEnter
        }
    }
}
