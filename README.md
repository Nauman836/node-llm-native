# node-llm-native

A native Node.js addon wrapper for `llama.cpp`.

This project exposes a simple JavaScript API for loading local GGUF models and generating text from Node.js. It builds the `llama.cpp` backend as shared libraries and compiles a native addon with `node-gyp`.

## Features

- Native Node.js addon wrapper around `llama.cpp`
- Simple `Model` / `createModel` interface
- Automatic backend build via `scripts/ensure-backend.js`
- Supports custom model path, device selection, GPU layers, context size, threads, and temperature

  > Note: Currently this project is under development and only supports cpu backend, so make sure that while using createModel class set the device to 'cpu' or 'auto'.

- Example usage included in `example.js`

## Project Structure

- `index.js` - JavaScript wrapper and API entrypoint
- `addon/` - native addon C++ sources
- `model/` - C++ model sources
- `binding.gyp` - Node addon build configuration
- `scripts/ensure-backend.js` - builds llama.cpp shared libs if missing
- `scripts/prerequisites.js` - ensures that build tools are available
- `vendor/llama.cpp/` - llama.cpp git submodule
- `test/smoke.js` - basic smoke test for the addon
- `example/example.js` - usage example
- `package.json` - package metadata and install scripts

## Prerequisites

Before installing, make sure your system has:

- Node.js (recommended >= 18)
- `npm`
- `node-gyp`
- `cmake`
- A C/C++ build toolchain (`gcc`, `g++`, `make`) or platform equivalent
- `python` if required by your `node-gyp` toolchain

## Installation

### From GitHub / Local source

Clone the repository and install dependencies:

```bash
git clone --recurse-submodules https://github.com/Nauman836/node-llm-native.git node-llm-native
cd node-llm-native
npm install
```

If you already cloned without submodules, run:

```bash
git submodule update --init --recursive
npm install
```

The `install` script runs `node scripts/ensure-backend.js && node-gyp rebuild`, which will:

1. build the `llama.cpp` backend shared libraries if they are missing
2. compile the native addon `node_llm_native`

### From npm

> Note: Right now node-llm-native package is not available at npm. It is under development.

Once published to npm, users should be able to install directly:

```bash
npm install node-llm-native
```

> Note: This package currently contains native bindings and a bundled llama.cpp backend, so installation requires the native build toolchain above.

## Usage

Example from `example.js`:

```js
const { Model, createModel } = require('./');

const beginnerModel = new Model('model_path.gguf');

const advancedModel = createModel({
  model: 'model_path.gguf',
  device: 'auto',
  gpuLayers: -1,
  contextSize: 2048,
  threads: 4,
  temperature: 0.7
});

(async () => {
  await beginnerModel.load();
  const reply = await beginnerModel.generate('Hello, how are you?', 24);
  console.log('reply:', reply);

  await advancedModel.load();
  const response = await advancedModel.generate('Hello, how are you?', 24);
  console.log('reply:', reply);
})();
```

### API

- `new Model(options)`
  - `options.model` - required path to a GGUF model file
  - `options.device` - `'auto'` by default
  - `options.gpuLayers` - number of GPU layers, default `-1`
  - `options.contextSize` - default `2048`
  - `options.threads` - default `4`
  - `options.temperature` - default `0.7`

- `createModel(options)` - returns a new `Model` instance
- `model.load([modelPath])` - loads the model into memory
- `model.generate(prompt, maxTokens)` - generates text from a prompt
- `model.chat(prompt, maxTokens)` - alias for `generate`
- `model.getConfig()` - returns resolved options

## Building and Testing

To build manually:

```bash
npm run build
```

To run the smoke test:

```bash
npm run test
```