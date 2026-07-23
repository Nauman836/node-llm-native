const { spawnSync } = require("child_process");
const fs = require("fs");
const os = require("os");
const path = require("path");

const root = path.resolve(__dirname, "..");
const cppDir = path.join(root, "cpp-llm-native");
const buildDir = path.join(cppDir, "build");
const libDir = path.join(buildDir, "lib");

const required =
  process.platform === "win32"
    ? [path.join(libDir, "cpp_llm_native.lib")]
    : [path.join(libDir, "libcpp_llm_native.a")];

// Skip if already built
const missing = required.filter((file) => !fs.existsSync(file));

if (missing.length === 0) {
  console.log("✓ cpp-llm-native already built.");
  process.exit(0);
}

console.log("Building cpp-llm-native CPU backend...");

const configureArgs = [
  "-S",
  cppDir,
  "-B",
  buildDir,

  "-DCMAKE_POSITION_INDEPENDENT_CODE=ON",

  "-DGGML_CPU=ON",
  "-DGGML_CUDA=OFF",
  "-DGGML_METAL=OFF",
  "-DGGML_VULKAN=OFF",
  "-DGGML_HIP=OFF",
  "-DGGML_SYCL=OFF",
];

if (process.platform !== "win32") {
  configureArgs.push("-DCMAKE_BUILD_TYPE=Release");
}

const configure = spawnSync("cmake", configureArgs, {
  cwd: root,
  stdio: "inherit",
});

if (configure.status !== 0) {
  process.exit(configure.status || 1);
}

const buildArgs = [
  "--build",
  buildDir,
  "--parallel",
  os.cpus().length.toString(),
];

if (process.platform === "win32") {
  buildArgs.splice(2, 0, "--config", "Release");
}

const build = spawnSync("cmake", buildArgs, {
  cwd: root,
  stdio: "inherit",
});

if (build.status !== 0) {
  process.exit(build.status || 1);
}

const stillMissing = required.filter((file) => !fs.existsSync(file));

if (stillMissing.length) {
  console.error("\nFailed to build cpp-llm-native:\n");

  for (const file of stillMissing) {
    console.error("  " + file);
  }

  process.exit(1);
}

console.log("✓ cpp-llm-native CPU backend built successfully.");