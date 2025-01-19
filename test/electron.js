const { app, ipcMain, BrowserWindow } = require('electron');
const path = require('path');
const swift = require('swift');

function createWindow() {
    const win = new BrowserWindow({
        width: 300,
        height: 200,
        webPreferences: {
            preload: path.join(__dirname, 'preload.js')
        }
    });

    // Load a simple HTML file that has a button to show the SwiftUI window
    win.loadFile(path.join(__dirname, 'index.html'));
}

app.on('window-all-closed', () => {
    if (process.platform !== 'darwin') {
        app.quit();
    }
});

app.on('activate', () => {
    if (BrowserWindow.getAllWindows().length === 0) {
        createWindow();
    }
});

async function main() {
  createWindow();
}

ipcMain.on('helloGui', () => {
  console.log(swift.helloWorld("Hello from Node!"));
  swift.helloGui();
});

app.whenReady().then(main);

