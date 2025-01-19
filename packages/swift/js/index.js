const EventEmitter = require('events');
const addon = require('../build/Release/swift_addon');

class SwiftAddon extends EventEmitter {
    constructor() {
        super();
        this.addon = new addon.SwiftAddon();

        this.addon.on('todoAdded', (payload) => {
            this.emit('todoAdded', this.#fromSwift(payload));
        });

        this.addon.on('todoUpdated', (payload) => {
            this.emit('todoUpdated', this.#fromSwift(payload));
        });

        this.addon.on('todoDeleted', (payload) => {
            this.emit('todoDeleted', this.#fromSwift(payload));
        });
    }

    helloWorld(input = "") {
      this.addon.helloWorld(input);
    }

    helloGui() {
      this.addon.helloGui();
    }

    #fromSwift(payload) {
      const parsed = JSON.parse(payload);

      return { ...parsed, date: new Date(parsed.date * 1000) };
    }
}

module.exports = new SwiftAddon();
