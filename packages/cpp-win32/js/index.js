const EventEmitter = require('events');

class CppWin32Addon extends EventEmitter {
    constructor() {
        super();

        if (process.platform !== 'win32') {
          throw new Error('This module is only available on Windows');
        }
  
        const native = require('../build/Release/cpp_addon');
        console.log(native);
        this.addon = new native.CppWin32Addon();

        this.addon.on('todoAdded', (payload) => {
            this.emit('todoAdded', this.#parse(payload));
        });

        this.addon.on('todoUpdated', (payload) => {
            this.emit('todoUpdated', this.#parse(payload));
        });

        this.addon.on('todoDeleted', (payload) => {
            this.emit('todoDeleted', this.#parse(payload));
        });
    }

    helloWorld(input = "") {
      this.addon.helloWorld(input);
    }

    helloGui() {
      this.addon.helloGui();
    }

    #parse(payload) {
      const parsed = JSON.parse(payload);

      return { ...parsed, date: new Date(parsed.date) };
    }
}

if (process.platform === 'win32') {
  module.exports = new CppWin32Addon();
} else {
  module.exports = {};
}
