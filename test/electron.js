const { app, ipcMain, BrowserWindow } = require('electron');
const path = require('path');
const swift = require('swift');

let win;

function createWindow() {
  win = new BrowserWindow({
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
  setupSwiftListeners();
}

function setupSwiftListeners() {
  swift.on('todoAdded', (todo) => {
    console.log('Todo added:', todo);
  });

  swift.on('todoUpdated', (todo) => {
    console.log('Todo updated:', todo);
  });

  swift.on('todoDeleted', (todoId) => {
    console.log('Todo deleted:', todoId);
  });
}

ipcMain.on('helloGui', () => {
  swift.helloGui();
});

app.whenReady().then(main);

