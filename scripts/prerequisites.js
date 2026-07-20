const {spawnSync} = require("child_process")

const cmakeCheck = spawnSync("cmake", ["--version"], {
  stdio: "ignore",
});

if (cmakeCheck.error) {
  console.error(
    "CMake is required to build the llama.cpp backend. Please install CMake and try again."
  );
  process.exit(1);
}