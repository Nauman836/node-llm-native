const pkg = require("../");

if (!pkg || typeof pkg.createModel !== "function") {
  console.error("Native addon is not built");
  process.exit(1);
}

console.log(pkg.buildInfo());