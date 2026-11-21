// Resolve Wasm CLI paths against the Node working directory. Default saves stay in MEMFS.
if (ENVIRONMENT_IS_NODE) {
  Module['preRun'] = [() => {
    FS.mkdir('/work');
    FS.mount(NODEFS, {root: process.cwd()}, '/work');
    FS.chdir('/work');
    ENV['XDG_DATA_HOME'] = '/tmp';
  }];
}
