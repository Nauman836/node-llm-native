const addon = require('../build/Release/node_llm_native');

if (!addon || typeof addon.createModel !== 'function') {
  console.error('Native addon is not built');
  process.exit(1);
}

const model = addon.createModel({
  model: './MiniCPM5-1B-Claude-Opus-Fable5-V2-Thinking-Q8_0.gguf',
  device: 'auto',
  gpuLayers: -1,
  contextSize: 2048,
  threads: 4,
  temperature: 0.7
});

console.log('created', typeof model);
