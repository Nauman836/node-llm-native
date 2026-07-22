const fs = require("fs");
const path = require("path");

const backendDir = path.resolve(
  __dirname,
  "vendor",
  "llama.cpp",
  "build",
  "bin",
);
const delimiter = process.platform === "win32" ? ";" : ":";

if (process.platform === "linux" && fs.existsSync(backendDir)) {
  const current = process.env.LD_LIBRARY_PATH
    ? process.env.LD_LIBRARY_PATH.split(delimiter)
    : [];
  if (!current.includes(backendDir)) {
    process.env.LD_LIBRARY_PATH = [backendDir, ...current].join(delimiter);
  }
}

const addon = require("./build/Release/node_llm_native");

function normalizeOptions(options = {}) {
  if (typeof options === "string") {
    return { model: options };
  }

  if (!options || typeof options !== "object") {
    throw new TypeError(
      "Model options must be an object or a model path string",
    );
  }

  return {
    model: options.model,
    device: options.device || "auto",
    gpuLayers: options.gpuLayers ?? -1,
    contextSize: options.contextSize || 2048,
    threads: options.threads || 4,
    temperature: options.temperature ?? 0.7,
    ...options,
  };
}

class Model {
  constructor(options = {}) {
    const normalized = normalizeOptions(options);

    if (!normalized.model) {
      throw new Error("A model path is required");
    }

    this.options = normalized;
    this.instance = addon.createModel(normalized);
    this.loaded = false;
  }

  async load(modelPath = this.options.model) {
    const ok = this.instance.load(modelPath);
    this.loaded = ok;
    return ok;
  }

  async generate(prompt, maxTokens = 32) {
    if (!this.loaded) {
      const loaded = await this.load();
      if (!loaded) {
        throw new Error("Model could not be loaded");
      }
    }

    return this.instance.generate(prompt, maxTokens);
  }

  async chat(prompt, maxTokens = 64) {
    return this.generate(prompt, maxTokens);
  }

  getConfig() {
    return { ...this.options };
  }
}

function createModel(options = {}) {
  return new Model(options);
}

module.exports = { Model, createModel, buildInfo: addon.buildInfo };
