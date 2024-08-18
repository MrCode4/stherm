import QtQuick
import QtQuick.Controls

/*! ***********************************************************************************************
 * Built-in StackView keeps all the previous pages in memory and so all the bindings in all the
 * previous pages are running (including the timers and objects). The purpose of this SimpleStackView
 * is to replace the built-in StackView as is and solve those problems.
 * ***********************************************************************************************/
Item {
    id: root    
    readonly property int depth: loader.stack.length
    readonly property Item currentItem: loader.item
    property StackView parentStackView

    function push(page, props) {
        loader.stack.push({'page': page, 'props': props});
        loader.load(page, props);
    }

    function updateProps(page, newProps) {
        for(let i = 0; i < loader.stack.length; i++) {
            let vm = loader.stack[i];
            if (vm.page == page) {
                Object.assign(vm.props, newProps);
                break;
            }
        }
    }

    function pop() {
        if (loader.stack.length > 1) {
            loader.stack.splice(-1, 1);
            let vm = loader.stack[loader.stack.length-1];
            loader.load(vm.page, vm.props)
        }
        else {
            if (parentStackView) {
                parentStackView.pop();
            }
        }
    }

    Loader {
        id: loader
        property var stack: []

        asynchronous: false
        anchors.fill: parent

        function load(page, props) {
            loader.setSource(page, props)
        }
    }
}
