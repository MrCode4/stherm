import QtQuick
import Stherm

/*! ***********************************************************************************************
 * ToastManager: Manages registered requests for displing toast messages
 * ************************************************************************************************/

Item {
    id:root

    //Holds active toat messages requested
    property var toastsQueue:[]

    //UI component for displaying toasts messages
    property Toast toastComponent

    //Components request to display toast messages through this signal
    signal showToast(string message, string detail)

    //Manages duration of displying toasts and also triggers displaying other toasts in order
    Timer{
        id:timer
        interval:3000;
        running:true;
        repeat:false;
        onTriggered:{
            toastsQueue.shift();
            toastComponent.close();
            displayNextToast();
        }
    }

    //everytime this signal is fired, a requested toast message needs to be shown
    onShowToast: (message, detail) => makeToast(message, detail)

    //Handles adding requested toast messages to the queue and displaying them
    function makeToast(message:string, detail: string){
        toastsQueue.push({ "message": message, "detail": detail });

        if(toastsQueue.length===1){
            displayNextToast();
        }
    }

    //Displays all requested toast messages in order
    function displayNextToast(){
        if(toastsQueue.length>0){
            var currentToast = toastsQueue[0];
            toastComponent.message = currentToast.message;
            toastComponent.detail = currentToast.detail;
            toastComponent.open();
            timer.start();
        }
    }
}
