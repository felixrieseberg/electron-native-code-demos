const EventEmitter = require('events');

class CppLinuxAddon extends EventEmitter {
  constructor() {
    super();

    if (process.platform !== 'linux') {
      throw new Error('This module is only available on Linux');
    }

    const native = require('bindings')('cpp_addon');
    this.addon = new native.CppLinuxAddon();

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
    return this.addon.helloGui();
  }

  #parse(payload) {
    const parsed = JSON.parse(payload);

    return { ...parsed, date: new Date(parsed.date) };
  }
}

if (process.platform === 'linux') {
  module.exports = new CppLinuxAddon();
} else {
  module.exports = {};
}

