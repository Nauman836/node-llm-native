# node-llm-native

> Run GGUF language models directly from Node.js using the power of **llama.cpp**.

`node-llm-native` is a native Node.js addon that lets you run local large language models with a clean JavaScript API while keeping the performance of C++ and `llama.cpp`.

The project is built with **CMake** and is designed to support multiple inference backends including **CPU**, **CUDA**, **Vulkan**, **Metal**, **HIP**, and **SYCL**.

> 🚧 **Status**
>
> The project is under active development.
>
> Currently only the **CPU backend** is supported.

---

# Features

- 🚀 Native Node.js addon (C++)
- ⚡ Powered by llama.cpp
- 📦 Install from npm
- 🤖 Supports GGUF models
- 💬 Chat API
- ✍️ Text completion API
- 🔄 Automatic model loading
- ⚙️ Configurable runtime options
- 🔍 Runtime backend information
- 🌍 Cross-platform
  - Linux
  - Windows
  - macOS

---

# Roadmap

## Available

- ✅ CPU

## Planned

- CUDA
- Vulkan
- Metal
- HIP
- SYCL

---

# Installation

```bash
npm install node-llm-native
```

---

# Build From Source

## Requirements

- Node.js 18+
- npm
- CMake 3.20+
- C++17 compiler

### Linux

- gcc
- g++
- make

### Windows

- Visual Studio 2022 (Desktop Development with C++)

### macOS

- Xcode Command Line Tools

Clone the repository

```bash
git clone --recurse-submodules https://github.com/Nauman836/node-llm-native.git

cd node-llm-native
```

If you forgot the submodules

```bash
git submodule update --init --recursive
```

Install dependencies

```bash
npm install
```

Build

```bash
npm run build
```

The build process automatically builds

- llama.cpp
- cpp-llm-native
- Node.js addon

---

# Quick Start

The easiest way to use the library is to pass a model path.

```javascript
const { Model } = require("node-llm-native");

const model = new Model("model.gguf");

const response = await model.chat(
    "Explain JavaScript in one sentence.",
    128
);

console.log(response);
```

Notice that `load()` is optional.

The model is loaded automatically the first time you call `chat()` or `generate()`.

If you prefer, you can load it manually.

```javascript
await model.load();
```

---

# Advanced Usage

```javascript
const { createModel } = require("node-llm-native");

const model = createModel({
    model: "model.gguf",
    device: "auto",
    gpuLayers: -1,
    contextSize: 2048,
    threads: 4,
    temperature: 0.7
});

const response = await model.chat(
    "Write a short story.",
    256
);

console.log(response);
```

---

# Chat API

`chat()` accepts either a string or a conversation.

## Simple Prompt

```javascript
const reply = await model.chat(
    "Who created Linux?",
    128
);
```

## Multi-turn Conversation

```javascript
const reply = await model.chat([
    {
        role: "system",
        content: "You are a helpful assistant."
    },
    {
        role: "user",
        content: "Explain recursion."
    }
], 256);
```

Supported roles

- system
- user
- assistant

---

# Text Completion

```javascript
const text = await model.generate(
    "Once upon a time",
    128
);

console.log(text);
```

---

# Runtime Information

You can inspect the compiled backend at runtime.

```javascript
const { buildInfo } = require("node-llm-native");

console.log(buildInfo());
```

Example

```javascript
{
    version: "...",
    backend: "CPU",
    compiler: "...",
    ...
}
```

---

# Available Backends

```javascript
const { Model } = require("node-llm-native");

console.log(Model.getPrimaryBackend());
```

Example

```text
CPU
```

List every compiled backend

```javascript
console.log(Model.getBackends());
```

Example

```javascript
[
    {
        name: "CPU",
        description: "CPU backend",
        isCpu: true
    }
]
```

---

# API

## Constructor

Create a model using a path.

```javascript
const model = new Model("model.gguf");
```

Or using an options object.

```javascript
const model = new Model({
    model: "model.gguf",
    contextSize: 4096
});
```

You can also use

```javascript
const model = createModel(options);
```

---

# Options

| Option | Default | Description |
|----------|----------|-------------|
| model | required | GGUF model path |
| device | `"auto"` | Backend selection |
| gpuLayers | `-1` | GPU layers |
| contextSize | `2048` | Context window |
| threads | `4` | CPU threads |
| temperature | `0.7` | Sampling temperature |

---

# Methods

## load()

Loads the model manually.

```javascript
await model.load();
```

---

## chat()

Generate chat responses.

```javascript
await model.chat(prompt, maxTokens);
```

or

```javascript
await model.chat(messages, maxTokens);
```

---

## generate()

Generate text completion.

```javascript
await model.generate(prompt, maxTokens);
```

---

## getConfig()

Returns the resolved configuration.

```javascript
const config = model.getConfig();

console.log(config);
```

---

# Current Limitations

At the moment only the CPU backend is available.

Use

```javascript
device: "cpu"
```

or simply

```javascript
device: "auto"
```

GPU backends will be added in future releases.

---

# Example

Run the example

```bash
node example/example.js
```

---

# Tests

```bash
npm test
```

---

# Project Structure

```
node-llm-native
│
├── addon/
├── cpp-llm-native/
│   ├── include/
│   ├── src/
│   └── vendor/
│       └── llama.cpp/
│
├── example/
├── scripts/
├── test/
├── CMakeLists.txt
├── index.js
└── package.json
```

---

# License

MIT

---

# Acknowledgements

This project is built on top of the incredible work of the **llama.cpp** contributors.