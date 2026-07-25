const { Model, createModel, buildInfo } = require("../");

const MODEL_PATH =
  "/home/nauman/Desktop/CodingFiles/models/qwen2.5-1.5b-instruct-q4_k_m.gguf";

(async () => {
  try {
    console.log("=== Build Information ===");
    console.log(buildInfo());

    console.log("\nPrimary Backend:");
    console.log(Model.getPrimaryBackend());

    console.log("\nAvailable Backends:");
    console.log(Model.getBackends());

    console.log("\n==============================");
    console.log(" Beginner API");
    console.log("==============================");

    const beginner = new Model(MODEL_PATH);

    // No need to call load()
    const reply = await beginner.chat(
      "Hello! Tell me something interesting about JavaScript.",
      128
    );

    console.log("\nAssistant:");
    console.log(reply);

    console.log("\n==============================");
    console.log(" Advanced API");
    console.log("==============================");

    const advanced = createModel({
      model: MODEL_PATH,
      device: "auto",
      gpuLayers: -1,
      contextSize: 2048,
      threads: 4,
      temperature: 0.7,
    });

    await advanced.load();

    const messages = [
      {
        role: "system",
        content: "You are a helpful programming assistant.",
      },
      {
        role: "user",
        content: "Explain promises in JavaScript.",
      },
    ];

    const answer = await advanced.chat(messages, 256);

    console.log("\nAssistant:");
    console.log(answer);

    console.log("\nResolved Configuration:");
    console.log(advanced.getConfig());
  } catch (error) {
    console.error("Example failed:", error.message);
    process.exit(1);
  }
})();