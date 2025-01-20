const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('native', {
  swift: {
    helloWorld: (input) => {
      ipcRenderer.send('swift-hello-world', input);
    },
    helloGui: () => {
      ipcRenderer.send('swift-hello-gui');
    }
  },
  'objective-c': {
    helloWorld: (input) => {
      ipcRenderer.send('objective-c-hello-world', input);
    },
    helloGui: () => {
      ipcRenderer.send('objective-c-hello-gui');
    }
  },
  cppWin32: {
    helloWorld: (input) => {
      ipcRenderer.send('cpp-win32-hello-world', input);
    },
    helloGui: () => {
      ipcRenderer.send('cpp-win32-hello-gui');
    }
  },
  platform: process.platform
});

document.addEventListener('DOMContentLoaded', () => {
  const buttons = document.querySelectorAll('button[data-platform]');

  buttons.forEach(button => {
    button.disabled = button.dataset.platform !== process.platform;
  });
});
