import QtQuick

/// Extended Stack View
Item {
    id: root
    property var stack: []
    property alias currentItem: loader.item

    function push(page, props) {
        console.log('Pushing Page:', page)
        root.stack.push({'page': page, 'props': props});
        loader.load(page, props);
    }

    function pop() {
        if (root.stack.length > 1) {
            root.stack.splice(-1, 1);
            var vm = root.stack[root.stack.length-1];
            console.log('Popping Page:', vm.page)
            loader.load(vm.page, vm.props)
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
