const { app, BrowserWindow } = require('electron');
const path = require('path');

const { setupNativeListeners } = require('./native-listeners');

let win;

function createWindow() {
  win = new BrowserWindow({
    width: 870,
    height: 700,
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
  setupNativeListeners();
}

app.whenReady().then(main);

