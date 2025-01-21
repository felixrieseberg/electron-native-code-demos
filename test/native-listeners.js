const { ipcMain } = require('electron');

const languages = ['swift', 'objective-c', 'cpp-win32'];

// Require the addons available on the current platform.
const addons = {
  swift: process.platform === 'darwin' ? require('swift') : null,
  'objective-c': process.platform === 'darwin' ? require('objective-c') : null,
  'cpp-win32': process.platform === 'win32' ? require('cpp-win32') : null,
  'cpp-linux': process.platform === 'linux' ? require('cpp-linux') : null,
}

function setupNativeListeners() {
  for (const language of languages) {
    // Only handle the addons that are available for the current platform.
    if (!addons[language]) continue;

    ipcMain.handle(`${language}-hello-world`, (event, input) => {
      const result = addons[language].helloWorld(input);
      console.log(`${language} helloWorld() called:`, result);
      return result;
    });

    ipcMain.on(`${language}-hello-gui`, () => {
      addons[language].helloGui();
    });

    // Setup the JavaScript listeners. This is simply a demo to
    // show that you can get data back from the native code.
    addons[language].on('todoAdded', (todo) => {
        console.log(`${language} todo added:`, todo);
    });

    addons[language].on('todoUpdated', (todo) => {
        console.log(`${language} todo updated:`, todo);
    });

    addons[language].on('todoDeleted', (todoId) => {
        console.log(`${language} todo deleted:`, todoId);
    });
  }
}

module.exports = { setupNativeListeners };
