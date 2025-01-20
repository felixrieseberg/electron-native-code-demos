const { ipcMain } = require('electron');

const languages = ['swift', 'objective-c'];
const addons = {
  swift: require('swift'),
  'objective-c': require('objective-c')
}

function setupNativeListeners() {
  for (const language of ['swift', 'objective-c']) {
      ipcMain.on(`${language}-hello-world`, (input) => {
          console.log(`${language} hello world:`, addons[language].helloWorld(input));
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
