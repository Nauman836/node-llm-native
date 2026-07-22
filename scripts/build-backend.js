const { spawnSync } = require("child_process");
const fs = require("fs");
const os = require("os");
const path = require("path");

const root = path.resolve(__dirname, "..");
const backendDir = path.join(root, "vendor", "llama.cpp", "build");
const libDir = path.join(backendDir, "bin");

const required =
  process.platform === "win32"
    ? [
        path.join(libDir, "libllama.dll"),
        path.join(libDir, "libggml.dll"),
        path.join(libDir, "libggml-base.dll"),
        path.join(libDir, "libggml-cpu.dll"),
      ]
    : process.platform === "darwin"
    ? [
        path.join(libDir, "libllama.dylib"),
        path.join(libDir, "libggml.dylib"),
        path.join(libDir, "libggml-base.dylib"),
        path.join(libDir, "libggml-cpu.dylib"),
      ]
    : [
        path.join(libDir, "libllama.so"),
        path.join(libDir, "libggml.so"),
        path.join(libDir, "libggml-base.so"),
        path.join(libDir, "libggml-cpu.so"),
      ];

// Skip if already built
const missing = required.filter((f) => !fs.existsSync(f));

if (missing.length === 0) {
  console.log("✓ llama.cpp backend already built.");
  process.exit(0);
}

console.log("Building llama.cpp CPU backend...");

const configureArgs = [
  "-S",
  path.join(root, "vendor", "llama.cpp"),
  "-B",
  backendDir,
  "-DBUILD_SHARED_LIBS=ON",
  "-DGGML_CPU=ON",
  "-DGGML_CUDA=OFF",
  "-DGGML_METAL=OFF",
  "-DGGML_VULKAN=OFF",
  "-DGGML_HIP=OFF",
  "-DGGML_SYCL=OFF",
];

if (process.platform !== "win32") {
  configureArgs.push(
    "-DCMAKE_BUILD_TYPE=Release",
    "-DCMAKE_BUILD_RPATH=$ORIGIN",
    "-DCMAKE_INSTALL_RPATH=$ORIGIN",
    "-DCMAKE_BUILD_RPATH_USE_ORIGIN=ON"
  );
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
  backendDir,
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

const stillMissing = required.filter((f) => !fs.existsSync(f));

if (stillMissing.length) {
  console.error("\nFailed to build required llama.cpp libraries:\n");

  for (const file of stillMissing) {
    console.error("  " + file);
  }

  process.exit(1);
}

console.log("✓ llama.cpp CPU backend built successfully.");