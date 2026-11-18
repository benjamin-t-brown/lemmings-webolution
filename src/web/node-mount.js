// Node runs the same Wasm CLI. Resolve relative archive and replay arguments
// against its working directory. The default save directory stays in MEMFS.
if (ENVIRONMENT_IS_NODE) {
  Module['preRun'] = [() => {
    FS.mkdir('/work');
    FS.mount(NODEFS, {root: process.cwd()}, '/work');
    FS.chdir('/work');
    ENV['XDG_DATA_HOME'] = '/tmp';
  }];
}
