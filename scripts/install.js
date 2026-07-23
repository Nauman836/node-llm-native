const os = require("os");
const fs = require("fs");
const path = require("path");
const unzipper = require("unzipper");

const root = path.resolve(__dirname, "..");
const prebuiltDir = path.join(root, "prebuilt");
const zipFile = path.join(root, "package.zip");

async function install() {
  const platform = os.platform();
  const arch = os.arch();

  let asset;

  if (platform === "linux" && arch === "x64") {
    asset = "node-llm-native-linux-x64.zip";
  } else if (platform === "win32" && arch === "x64") {
    asset = "node-llm-native-windows-x64.zip";
  } else if (platform === "darwin" && arch === "arm64") {
    asset = "node-llm-native-macos-arm64.zip";
  } else {
    throw new Error(`Unsupported platform: ${platform}-${arch}`);
  }

  const addonName = platform === "win32"
    ? "node_llm_native.node"
    : "node_llm_native.node";

  const addonPath = path.join(prebuiltDir, addonName);

  if (fs.existsSync(addonPath)) {
    console.log("✓ Prebuilt binary already installed.");
    return;
  }

  fs.mkdirSync(prebuiltDir, { recursive: true });

  const version = require("../package.json").version;

  const url =
    `https://github.com/Nauman836/node-llm-native/releases/download/v${version}/${asset}`;

  console.log(`Downloading ${asset}...`);

  const response = await fetch(url);

  if (!response.ok) {
    throw new Error(
      `Failed to download prebuilt binary.\n` +
      `HTTP ${response.status}\n` +
      `${url}`
    );
  }

  const buffer = Buffer.from(await response.arrayBuffer());

  fs.writeFileSync(zipFile, buffer);

  console.log("Extracting...");

  try {
    await fs
      .createReadStream(zipFile)
      .pipe(
        unzipper.Extract({
          path: prebuiltDir,
        })
      )
      .promise();
  } finally {
    if (fs.existsSync(zipFile)) {
      fs.unlinkSync(zipFile);
    }
  }

  console.log("✓ node-llm-native installed successfully.");
}

install().catch((err) => {
  console.error(err.message || err);
  process.exit(1);
});