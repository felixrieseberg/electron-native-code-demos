const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('swift', {
    helloGui: () => {
        ipcRenderer.send('helloGui');
    }
});
