const { spawnSync } = require('child_process');
const fs = require('fs');
const path = require('path');

const root = path.resolve(__dirname, '..');
const backendDir = path.join(root, 'vendor', 'llama.cpp', 'build');
const libDir = path.join(backendDir, 'bin');
const required = [
  path.join(libDir, 'libllama.so'),
  path.join(libDir, 'libggml.so'),
  path.join(libDir, 'libggml-base.so')
];

const missing = required.filter((p) => !fs.existsSync(p));
if (missing.length === 0) {
  console.log('llama.cpp backend already available');
  process.exit(0);
}

console.log('Building llama.cpp backend for native addon...');
const cmake = spawnSync('cmake', ['-S', path.join(root, 'vendor', 'llama.cpp'), '-B', backendDir, '-DBUILD_SHARED_LIBS=ON'], {
  cwd: root,
  stdio: 'inherit'
});
if (cmake.status !== 0) {
  console.error('Failed to configure llama.cpp backend');
  process.exit(cmake.status || 1);
}

const build = spawnSync('cmake', ['--build', backendDir, '--config', 'Release', '--target', 'llama', 'ggml', 'ggml-base', '-j2'], {
  cwd: root,
  stdio: 'inherit'
});
if (build.status !== 0) {
  console.error('Failed to build llama.cpp backend');
  process.exit(build.status || 1);
}

console.log('llama.cpp backend built successfully');
