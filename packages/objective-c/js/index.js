const EventEmitter = require('events');
const addon = require('../build/Release/objectivec_addon');

class ObjectiveCAddon extends EventEmitter {
    constructor() {
        super();
        this.addon = new addon.ObjectiveCAddon();

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

module.exports = new ObjectiveCAddon();
