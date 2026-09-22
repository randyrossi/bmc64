#!/usr/bin/env node
// Runs the web UI's JavaScript tests.
//
//     node tools/webui_test/run_tests.mjs
//
// Needs Node.js 22.12 or newer (older versions cannot load the ES modules in
// src/webui/assets/js, which are plain .js files). `source get_gnu_toolchain.sh`
// puts a suitable Node.js on PATH. Exit status is 0 if every test passed.

const [major, minor] = process.versions.node.split(".").map(Number);
if (major < 22 || (major === 22 && minor < 12)) {
  console.error("Node.js 22.12 or newer is required (this is " + process.version + ").");
  console.error("Run: source get_gnu_toolchain.sh");
  process.exit(2);
}

const { runTests } = await import("./basic.test.js");
const { passed, failed, log } = runTests();
for (const line of log) console.log(line);
console.log(passed + " passed, " + failed + " failed");
process.exit(failed ? 1 : 0);
