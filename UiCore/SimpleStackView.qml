import QtQuick
import QtQuick.Controls

/// Extended Stack View
Item {
    id: root
    property var stack: []
    property int depth: stack.length
    property alias currentItem: loader.item
    property StackView parentStackView

    function push(page, props) {
        root.stack.push({'page': page, 'props': props});
        loader.load(page, props);
    }

    function pop() {
        if (root.stack.length > 1) {
            root.stack.splice(-1, 1);
            var vm = root.stack[root.stack.length-1];
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
        asynchronous: false
        anchors.fill: parent

        function load(page, props) {
            loader.setSource(page, props)
        }
    }
}
