const fs = require("fs");
const path = require("path");

const root = __dirname;

let addon;

const prebuiltAddon = path.join(root, "prebuilt", "node_llm_native.node");
const localAddon = path.join(root, "build", "Release", "node_llm_native.node");

if (fs.existsSync(prebuiltAddon)) {
  addon = require(prebuiltAddon);
} else if (fs.existsSync(localAddon)) {
  addon = require(localAddon);
} else {
  throw new Error(
    "node-llm-native binary not found. Run 'npm install'"
  );
}

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

  /**
   * Chat-style generation with message history.
   * @param {Array<{role: string, content: string}>} messages - Conversation messages
   * @param {number} maxTokens - Maximum tokens to generate
   * @returns {Promise<string>} - Assistant's response
   */
  async chat(messages, maxTokens = 512) {
    if (!this.loaded) {
      const loaded = await this.load();
      if (!loaded) {
        throw new Error("Model could not be loaded");
      }
    }

    // Normalize single string prompt to message array
    if (typeof messages === "string") {
      messages = [{ role: "user", content: messages }];
    }

    if (!Array.isArray(messages)) {
      throw new TypeError("chat() expects an array of messages or a string prompt");
    }

    // Validate message format
    for (const msg of messages) {
      if (!msg || typeof msg !== "object") {
        throw new TypeError("Each message must be an object with 'role' and 'content'");
      }
      if (!msg.role || typeof msg.role !== "string") {
        throw new TypeError("Each message must have a string 'role'");
      }
      if (msg.content === undefined || msg.content === null) {
        throw new TypeError("Each message must have a 'content' field");
      }
      // Convert content to string if needed
      msg.content = String(msg.content);
    }

    return this.instance.chat(messages, maxTokens);
  }

  getConfig() {
    return { ...this.options };
  }

  // ── Static backend info methods ──

  /**
   * Get the primary (preferred) backend name.
   * @returns {string} - e.g. "Metal", "CUDA", "CPU"
   */
  static getPrimaryBackend() {
    return addon.Model.getPrimaryBackend();
  }

  /**
   * Get all available backends.
   * @returns {Array<{name: string, description: string, isCpu: boolean}>}
   */
  static getBackends() {
    return addon.Model.getBackends();
  }
}

function createModel(options = {}) {
  return new Model(options);
}

// Build info with runtime backend detection
function buildInfo() {
  const info = addon.buildInfo();
  return info;
}

module.exports = { Model, createModel, buildInfo };