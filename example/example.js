const { Model, createModel } = require('../');

// Beginner-friendly: just pass a model path string
const beginnerModel = new Model('/home/nauman/Desktop/CodingFiles/models/MiniCPM5-1B-Claude-Opus-Fable5-V2-Thinking-Q8_0.gguf');

// Advanced: pass an options object for full control
const advancedModel = createModel({
  model: '/home/nauman/Desktop/CodingFiles/models/MiniCPM5-1B-Claude-Opus-Fable5-V2-Thinking-Q8_0.gguf',
  device: 'auto',
  gpuLayers: -1,
  contextSize: 2048,
  threads: 4,
  temperature: 0.7
});

(async () => {
  try {
    console.log('Loading model...');
    const beginnerLoaded = await beginnerModel.load();
    console.log('Model loaded:', beginnerLoaded ? 'yes' : 'no');

    const prompt = 'Hello, how are you?';
    const reply = await beginnerModel.generate(prompt, 24);
    console.log('\nPrompt:');
    console.log(prompt);
    console.log('\nModel reply:');
    console.log(reply);

    const advancedLoaded = await advancedModel.load();
    console.log('\nAdvanced model loaded:', advancedLoaded ? 'yes' : 'no');
    const jokePrompt = 'Tell me a short joke.';
    const jokeReply = await advancedModel.chat(jokePrompt, 20);
    console.log('\nPrompt:');
    console.log(jokePrompt);
    console.log('\nModel reply:');
    console.log(jokeReply);
  } catch (error) {
    console.error('Example failed:', error.message);
    process.exit(1);
  }
})();