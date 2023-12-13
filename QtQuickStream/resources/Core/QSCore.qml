import QtQuick
import QtQuickStream

QSCoreCpp {
    id: core

    /* Signal Handlers
     * ****************************************************************************************/
    Component.onCompleted: {
        // Init QSUtil singleton by simply calling it.
        QSUtil;
    }

    // Handle C++ backend signal to create a Repository
    onSigCreateRepo: (repoId, isRemote) => createRepo(repoId, isRemote);

    /* Functions
     * ****************************************************************************************/
    function createDefaultRepo(imports: object) {
        // Create repo
        console.log("creating repo")
        var newRepo = createRepo(core.coreId, false);
        newRepo.imports = imports;

        return newRepo;
    }

    function createRepo(repoId: string, isRemote: bool) {
        // Create repo
        console.log( repoId, isRemote)
        var newRepo = QSSerializer.createQSObject("QSRepository", ["QtQuickStream"], core);
        console.log("repo created")

        newRepo._qsUuid = repoId;
        newRepo.qsIsAvailable = !isRemote;

        console.log("adding repo")
        // Add repo
        addRepo(newRepo);

        return newRepo;
    }
}
