# node-llm-native

A high-performance native Node.js addon for running local GGUF language models using **llama.cpp**.

`node-llm-native` provides a simple JavaScript API while leveraging the performance of C++ and `llama.cpp` under the hood. The project is built with **CMake** and is designed to support multiple inference backends such as CPU, CUDA, Vulkan, Metal, HIP, and SYCL in the future.

> **Project Status**
>
> 🚧 This project is currently under active development.
>
> At the moment, only the **CPU backend** is supported.

---

# Features

- Native Node.js addon written in C++
- Powered by `llama.cpp`
- Simple JavaScript API
- Automatic native build using **CMake** and **cmake-js**
- Supports GGUF models
- Configurable:
  - Model path
  - Context size
  - Threads
  - GPU layers
  - Temperature
- Cross-platform
  - Linux
  - Windows
  - macOS

---

# Roadmap

## Current

- ✅ CPU backend

## Planned

- CUDA
- Vulkan
- Metal
- HIP
- SYCL

---

# Project Structure

```
node-llm-native/
│
├── addon/                 # Node.js native addon
├── cpp-llm-native/        # Native C++ wrapper library
│   ├── include/
│   ├── src/
│   └── vendor/
│       └── llama.cpp/
│
├── example/
├── test/
├── scripts/
├── CMakeLists.txt
├── package.json
└── index.js
```

---

# Prerequisites

Before building, install:

- Node.js 18+
- npm
- CMake 3.20+
- C++17 compiler

Linux

- gcc
- g++
- make

Windows

- Visual Studio 2022 (Desktop C++)

macOS

- Xcode Command Line Tools

---

# Installation

## Clone

```bash
git clone --recurse-submodules https://github.com/Nauman836/node-llm-native.git

cd node-llm-native
```

If you forgot the submodules:

```bash
git submodule update --init --recursive
```

Install dependencies:

```bash
npm install
```

---

# Build

```bash
npm run build
```

The build process automatically:

1. Configures CMake
2. Builds `llama.cpp`
3. Builds `cpp-llm-native`
4. Builds the Node.js addon

---

# Usage

```javascript
const { Model, createModel } = require("node-llm-native");

// Beginner API
const model = new Model("model.gguf");

await model.load();

const reply = await model.generate(
    "Hello, how are you?",
    32
);

console.log(reply);
```

Advanced API

```javascript
const model = createModel({
    model: "model.gguf",
    device: "cpu",
    gpuLayers: -1,
    contextSize: 2048,
    threads: 4,
    temperature: 0.7,
});

await model.load();

const reply = await model.generate(
    "Write a short story.",
    128
);

console.log(reply);
```

---

# API

## Model

### Constructor

```javascript
new Model(modelPath)
```

or

```javascript
createModel(options)
```

---

### Options

| Option | Default | Description |
|---------|----------|-------------|
| model | required | GGUF model path |
| device | `"auto"` | Backend device |
| gpuLayers | `-1` | GPU layers |
| contextSize | `2048` | Context window |
| threads | `4` | CPU threads |
| temperature | `0.7` | Sampling temperature |

---

### Methods

```javascript
await model.load()
```

Loads the model.

---

```javascript
await model.generate(prompt, maxTokens)
```

Generates text.

---

```javascript
await model.chat(prompt, maxTokens)
```

Alias of `generate()`.

---

```javascript
model.getConfig()
```

Returns the resolved configuration.

---

# Current Limitations

- CPU backend only
- CUDA support is under development
- Vulkan support is planned
- Metal support is planned
- HIP support is planned

When using `createModel()`, set:

```javascript
device: "cpu"
```

or

```javascript
device: "auto"
```

---

# Running the Example

```bash
node example/example.js
```

---

# Running Tests

```bash
npm test
```

---

# License

MIT

---

# Acknowledgements

This project is built on top of the amazing work done by the **llama.cpp** contributors.