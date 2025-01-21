const EventEmitter = require('events');

class SwiftAddon extends EventEmitter {
    constructor() {
        super();

        if (process.platform !== 'darwin') {
          throw new Error('This module is only available on macOS');
        }

        const native = require('bindings')('swift_addon');
        this.addon = new native.SwiftAddon();

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

if (process.platform === 'darwin') {
  module.exports = new SwiftAddon();
} else {
  module.exports = {};
}
