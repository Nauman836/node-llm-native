const { Model, createModel } = require('.');

// Beginner-friendly: just pass a model path string
const beginnerModel = new Model('./MiniCPM5-1B-Claude-Opus-Fable5-V2-Thinking-Q8_0.gguf');

// Advanced: pass an options object for full control
const advancedModel = createModel({
  model: './MiniCPM5-1B-Claude-Opus-Fable5-V2-Thinking-Q8_0.gguf',
  device: 'auto',
  gpuLayers: -1,
  contextSize: 2048,
  threads: 4,
  temperature: 0.7
});

(async () => {
  const beginnerLoaded = await beginnerModel.load();
  const reply = await beginnerModel.generate('Hello, how are you?', 24);
  console.log('beginner reply:', reply);

  const advancedLoaded = await advancedModel.load();
  console.log('advanced config:', advancedModel.getConfig());
  console.log('advanced reply:', await advancedModel.chat('Tell me a short joke.', 20));
})();