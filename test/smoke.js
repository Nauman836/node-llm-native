const pkg = require('../');

if (!pkg || typeof pkg.createModel !== 'function') {
  console.error('Native addon is not built');
  process.exit(1);
}

const model = pkg.createModel({
  model: '/home/nauman/Desktop/CodingFiles/models/MiniCPM5-1B-Claude-Opus-Fable5-V2-Thinking-Q8_0.gguf',
  device: 'auto',
  gpuLayers: -1,
  contextSize: 2048,
  threads: 4,
  temperature: 0.7
});

console.log('created', typeof model);
