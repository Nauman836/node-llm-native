# node-llm

A lightweight Node.js wrapper for `llama.cpp` using native Node.js addons.

This project exposes a simple JavaScript API for loading local GGUF models and generating text from Node.js. It builds `llama.cpp` as a shared backend, then compiles a native addon with `node-gyp`.

## Features

- Native Node.js addon wrapper around `llama.cpp`
- Simple `Model` / `createModel` interface
- Automatic backend build via `scripts/ensure-backend.js`
- Supports custom model path, device selection, GPU layers, context size, threads, and temperature
- Example usage included in `example-js-api.js`

## Project Structure

- `index.js` - JavaScript wrapper and API entrypoint
- `src/` - native addon C++ sources
- `binding.gyp` - Node addon build configuration
- `scripts/ensure-backend.js` - builds llama.cpp shared libs if missing
- `vendor/llama.cpp/` - bundled llama.cpp source tree
- `test/smoke.js` - basic smoke test for the addon
- `example-js-api.js` - usage example
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
git clone <your-repository-url> node-llm-native
cd node-llm-native
npm install
```

The `install` script runs `node scripts/ensure-backend.js && node-gyp rebuild`, which will:

1. build the bundled `llama.cpp` backend shared libraries if they are missing
2. compile the native addon `node_llm_native`

### From npm

Once published to npm, users should be able to install directly:

```bash
npm install node-llm
```

> Note: This package currently contains native bindings and a bundled llama.cpp backend, so installation requires the native build toolchain above.

## Usage

Example from `example-js-api.js`:

```js
const { Model, createModel } = require('./');

const beginnerModel = new Model('./MiniCPM5-1B-Claude-Opus-Fable5-V2-Thinking-Q8_0.gguf');

const advancedModel = createModel({
  model: './MiniCPM5-1B-Claude-Opus-Fable5-V2-Thinking-Q8_0.gguf',
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
npm run prepare
```

To run the smoke test:

```bash
npm test
```

## Publishing to npm

When you are ready to publish:

1. Ensure `package.json` contains a unique package `name`, `version`, and metadata fields such as `author`, `repository`, `license`, and `keywords`.
2. Log in to npm:

```bash
npm login
```

3. Publish the package:

```bash
npm publish --access public
```

If you want to publish a new version later, update `package.json` `version` and run `npm publish` again.

## GitHub Push Guide

If the repository is not initialized yet:

```bash
git init
git add .
git commit -m "Initial commit"
git branch -M main
git remote add origin https://github.com/<username>/<repo>.git
git push -u origin main
```

Recommended `.gitignore` entries:

```
node_modules/
build/
vendor/llama.cpp/build/
*.log
```

## Review of the Project

This project is a strong foundation for a Node.js wrapper around `llama.cpp`.

What works well:

- Simple wrapper API with both `Model` and `createModel`
- Automatic backend build from bundled `vendor/llama.cpp`
- Native addon integration through `node-gyp`
- A clean example and smoke test to verify the addon is built

Areas to improve before publishing:

- Add package metadata in `package.json` (`author`, `repository`, `bugs`, `homepage`, `keywords`)
- Add root-level `README.md` (this file) for GitHub and npm users
- Add a `.gitignore` to avoid committing build artifacts
- Clarify supported Node versions and platform compatibility
- Expand tests beyond the smoke test to cover model loading and generation
- Consider shipping prebuilt binaries or a fallback for users without a full native toolchain

## Notes

- The project expects local GGUF model files such as `MiniCPM5-1B-Claude-Opus-Fable5-V2-Thinking-Q8_0.gguf`.
- Model path resolution also checks the current working directory and the source tree.
- The native addon depends on `libllama.so`, `libggml.so`, and `libggml-base.so` being built into `vendor/llama.cpp/build/bin`.

---

If you want, I can also help you update `package.json` for npm publishing and add a `.gitignore` file for this project.