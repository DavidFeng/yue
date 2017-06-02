#!/usr/bin/env node

// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {targetCpu, execSync} = require('./common')

execSync(`node ./scripts/bootstrap.js --target-cpu=${targetCpu}`)

// Build common targets.
const targets = [
  "libnativeui",
  'lua_yue',
  'yue',
]
execSync(`node ./scripts/build.js out/Release ${targets.join(' ')}`)

// Build node extensions.
const runtimes = {
  node: [
    'v8.0.0',
    'v7.0.0',
  ],
  electron: [
    'v1.7.0',
    'v1.6.0',
  ],
}
for (let runtime in runtimes) {
  for (let nodever of runtimes[runtime])
    execSync(`node ./scripts/create_node_extension.js --target-cpu=${targetCpu} ${runtime} ${nodever}`)
}

// Build and run tests.
if (targetCpu == 'x64') {
  const tests = [
    'nativeui_unittests',
    'lua_unittests',
  ]
  execSync(`node ./scripts/build.js out/Debug ${tests.join(' ')}`)

  for (test of tests)
    execSync(`./out/Debug/${test} --single-process-tests`)
}