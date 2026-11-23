import assert from 'node:assert/strict';
import {createHash} from 'node:crypto';
import {access, open, readFile} from 'node:fs/promises';
import {createRequire} from 'node:module';
import {dirname, resolve} from 'node:path';
import {runInNewContext} from 'node:vm';
import {
  createGameData, extractDiscPatch, mountGameData, readCdImage, selectInstalledFiles,
} from '../web/web-game-data.mjs';

const expectedBase = '74b544beee769b0354e64b25eaec06fe725d50d2df11a94270600ae03a7e6a51';
const expectedPatch10 = 'fd6025ddc5520fbf5c36314f602b9321ca1f088f391800722f85e0dd6ddecf1e';
const expectedPatch11 = 'a393a45e71d797fc5b646a9a235d1f64298129f378c6f4b79f47ec773d00de53';
const encoder = new TextEncoder();
const blockSize = 2048;

async function checkMobilePage(shellPath) {
  const html = await readFile(shellPath, 'utf8');
  const notice = html.match(/<aside id="mobile-warning"[^>]*\bhidden>([\s\S]*?)<\/aside>/);
  assert.ok(notice);
  assert.match(notice[1], /<p>This game does not play well on mobile\. Play at your own discretion\.<\/p>/);
  assert.match(notice[1], /<button id="dismiss-mobile-warning" type="button">Dismiss<\/button>/);
  assert.ok(notice.index < html.indexOf('id="setup"'));
  assert.ok(notice.index < html.indexOf('id="start"'));
  assert.match(html, /<canvas id="canvas" width="640" height="480"/);
  assert.match(html, /<p id="rotate-phone" role="status" hidden>Turn your phone sideways to play\. The game keeps running\.<\/p>/);
  assert.match(html, /html\.phone-play, \.phone-play body\s*\{[^}]*overflow: hidden; overscroll-behavior: none;/);
  assert.match(html, /\.phone-play main\s*\{[^}]*position: fixed; inset: 0;[^}]*place-items: center;[^}]*touch-action: none;[^}]*user-select: none;[^}]*-webkit-touch-callout: none;/);
  assert.match(html, /\.phone-play main > :not\(#canvas, #rotate-phone\)\s*\{ display: none; \}/);
  assert.match(html, /\.phone-play #canvas\s*\{[^}]*width: min\(100dvw, calc\(100dvh \* 4 \/ 3\)\);/);
  assert.match(html, /\.phone-play #canvas\[inert\]\s*\{ pointer-events: none; \}/);
  assert.match(html, /\.phone-play #rotate-phone:not\(\[hidden\]\)\s*\{[^}]*position: absolute; inset: 0; z-index: 1;/);
  const [, setupViewport] = html.match(/<meta name="viewport" content="([^"]+)">/);
  assert.equal(setupViewport, 'width=device-width, initial-scale=1');
  const [, script] = html.match(/<script>([\s\S]*?)<\/script>/);

  const noStorage = () => { throw new Error('Storage unavailable'); };
  class MouseEvent extends Event {
    constructor(type, fields) {
      super(type, {cancelable: true});
      Object.assign(this, fields);
    }
  }
  function visit({coarse = false, fine = false, userAgentData, getStorage = noStorage} = {}) {
    const viewport = {content: setupViewport};
    const classes = new Set();
    const elements = new Map([['meta[name="viewport"]', viewport]]);
    const windowEvents = new EventTarget();
    const portrait = Object.assign(new EventTarget(), {matches: true});
    const queries = new Map([
      ['(pointer: coarse)', {matches: coarse}], ['(any-pointer: fine)', {matches: fine}],
      ['(orientation: portrait)', portrait],
    ]);
    const document = {
      documentElement: {classList: {add: name => classes.add(name), remove: name => classes.delete(name)}},
      querySelector(selector) {
        if (!elements.has(selector)) elements.set(selector, Object.assign(new EventTarget(), {
          hidden: true, inert: false,
          focus() { document.activeElement = this; },
        }));
        return elements.get(selector);
      },
      hasFocus: () => true,
      createElement: () => ({}),
      head: {append(script) { gameScript = script.src; }},
    };
    let gameScript;
    const browser = {
      navigator: {userAgentData},
      matchMedia(query) {
        assert.ok(queries.has(query), `Unexpected media query ${query}`);
        return queries.get(query);
      },
      get localStorage() { return getStorage(); },
      URL, location: {href: 'https://example.test/game/'}, crossOriginIsolated: true,
      sessionStorage: {removeItem() {}},
      document, screen: {}, Event, MouseEvent,
      addEventListener: windowEvents.addEventListener.bind(windowEvents),
      dispatchEvent: windowEvents.dispatchEvent.bind(windowEvents),
    };
    browser.window = browser;
    runInNewContext(script, browser, {filename: 'webolution.html'});
    assert.equal(gameScript, './webolution.js');
    assert.equal(typeof elements.get('#start').onclick, 'function');
    return {notice: elements.get('#mobile-warning'), dismiss: elements.get('#dismiss-mobile-warning'),
      start: elements.get('#start'), canvas: elements.get('#canvas'), prompt: elements.get('#rotate-phone'),
      main: elements.get('main'), browser, document, viewport, classes,
      turn(matches) {
        portrait.matches = matches;
        portrait.dispatchEvent(new Event('change'));
      }};
  }

  assert.equal(visit({coarse: true}).notice.hidden, false);
  assert.equal(visit({fine: true}).notice.hidden, true);
  assert.equal(visit({coarse: true, fine: true, userAgentData: {mobile: false}}).notice.hidden, true);
  assert.equal(visit({userAgentData: {mobile: true}}).notice.hidden, false);
  console.log('PASS mobile warning detection on phone, desktop, touch laptop and mobile user agent data');

  const values = new Map();
  const storage = {getItem: key => values.get(key), setItem: (key, value) => values.set(key, value)};
  for (const [getStorage, hiddenOnReturn] of [
    [() => storage, true],
    [() => ({getItem: noStorage, setItem: noStorage}), false],
    [noStorage, false],
  ]) {
    const phone = {coarse: true, getStorage};
    const {notice, dismiss} = visit(phone);
    assert.equal(notice.hidden, false);
    dismiss.onclick();
    assert.equal(notice.hidden, true);
    assert.equal(visit(phone).notice.hidden, hiddenOnReturn);
  }
  console.log('PASS mobile warning dismissal with working storage, throwing methods and a throwing getter');

  async function startGame({browser, start}, onMain = () => {}) {
    const {Module, navigator} = browser;
    let close;
    Module.ccall = name => {
      assert.equal(name, 'main');
      onMain();
      return new Promise(resolve => { close = resolve; });
    };
    Module.FS = {mkdir() {}, mount() {}, syncfs(read, done) { done(); }};
    navigator.locks = {request: async (name, options, run) => run({})};
    const saveLock = runInNewContext('loadSaveAndOfferStart([])', browser);
    await new Promise(setImmediate);
    assert.equal(start.hidden, false);
    const finished = start.onclick();
    assert.equal(typeof close, 'function', 'main starts in the click handler');
    return async () => {
      close(0);
      await finished;
      await saveLock;
    };
  }

  const phone = visit({coarse: true});
  const calls = [];
  let allowFullscreen;
  phone.main.requestFullscreen = () => {
    calls.push('fullscreen');
    return new Promise(resolve => { allowFullscreen = resolve; });
  };
  phone.document.exitFullscreen = async () => {
    calls.push('exit fullscreen');
    phone.document.fullscreenElement = null;
  };
  phone.browser.screen.orientation = {
    async lock(mode) {
      assert.equal(phone.document.fullscreenElement, phone.main);
      calls.push(mode);
      throw new Error('Orientation lock denied');
    },
    unlock() { calls.push('unlock'); },
  };
  phone.browser.addEventListener('blur', () => calls.push('blur'));
  phone.browser.addEventListener('focus', () => calls.push('focus'));
  assert.equal(phone.prompt.hidden, true);
  assert.equal(phone.viewport.content, setupViewport);
  const endPhoneGame = await startGame(phone, () => calls.push('main'));
  assert.deepEqual(calls, ['blur', 'fullscreen', 'main']);
  assert.equal(phone.notice.hidden, true);
  assert.equal(phone.canvas.hidden, false);
  assert.equal(phone.prompt.hidden, false);
  assert.equal(phone.canvas.inert, true);
  assert.ok(phone.classes.has('phone-play'));
  assert.equal(phone.viewport.content, setupViewport + ', maximum-scale=1, user-scalable=no');
  phone.document.fullscreenElement = phone.main;
  allowFullscreen();
  await new Promise(setImmediate);
  assert.deepEqual(calls, ['blur', 'fullscreen', 'main', 'landscape']);

  let receivedPointer;
  phone.canvas.addEventListener('mousemove', event => { receivedPointer = [event.clientX, event.clientY]; });
  function pressCanvas() {
    const press = new MouseEvent('mousedown', {clientX: 360, clientY: 180, buttons: 1});
    // Node has no DOM capture tree. Send capture to window before delivering to the canvas.
    if (phone.browser.dispatchEvent(press)) phone.canvas.dispatchEvent(press);
    return press.defaultPrevented;
  }
  assert.equal(pressCanvas(), true);
  assert.equal(receivedPointer, undefined);
  for (const type of ['touchstart', 'touchmove', 'keydown', 'wheel']) {
    assert.equal(phone.browser.dispatchEvent(new Event(type, {cancelable: true})), false);
  }
  assert.equal(phone.browser.dispatchEvent(new Event('touchend', {cancelable: true})), true);
  phone.turn(false);
  assert.equal(phone.prompt.hidden, true);
  assert.equal(phone.canvas.inert, false);
  assert.equal(phone.document.activeElement, phone.canvas);
  assert.equal(calls.at(-1), 'focus');
  assert.equal(pressCanvas(), false);
  assert.deepEqual(receivedPointer, [360, 180]);
  assert.equal(phone.canvas.dispatchEvent(new Event('contextmenu', {cancelable: true})), false);
  phone.turn(true);
  assert.equal(phone.prompt.hidden, false);
  assert.equal(phone.canvas.inert, true);
  assert.equal(calls.at(-1), 'blur');
  await endPhoneGame();
  assert.deepEqual(calls, ['blur', 'fullscreen', 'main', 'landscape', 'focus', 'blur', 'unlock', 'exit fullscreen']);
  assert.equal(phone.canvas.hidden, true);
  assert.equal(phone.prompt.hidden, true);
  assert.equal(phone.canvas.inert, false);
  assert.equal(phone.classes.size, 0);
  assert.equal(phone.viewport.content, setupViewport);
  console.log('PASS phone fullscreen gesture, rejected landscape lock, portrait input gate and landscape restore');

  const manualPhone = visit({userAgentData: {mobile: true}});
  manualPhone.turn(false);
  const endManualGame = await startGame(manualPhone);
  assert.equal(manualPhone.prompt.hidden, true);
  assert.ok(manualPhone.classes.has('phone-play'));
  manualPhone.turn(true);
  assert.equal(manualPhone.prompt.hidden, false);
  assert.equal(manualPhone.canvas.inert, true);
  await endManualGame();
  console.log('PASS manual phone rotation without fullscreen or orientation APIs');

  for (const coarse of [false, true]) {
    const desktop = visit({coarse, fine: true});
    let fullscreenCalls = 0;
    let lockCalls = 0;
    desktop.main.requestFullscreen = () => { ++fullscreenCalls; };
    desktop.browser.screen.orientation = {lock() { ++lockCalls; }};
    const endDesktopGame = await startGame(desktop);
    desktop.turn(false);
    desktop.turn(true);
    assert.equal(desktop.classes.size, 0);
    assert.equal(desktop.viewport.content, setupViewport);
    assert.equal(desktop.document.activeElement, desktop.canvas);
    assert.equal(desktop.prompt.hidden, true);
    assert.equal(desktop.canvas.inert, false);
    assert.equal(desktop.browser.dispatchEvent(new Event('touchstart', {cancelable: true})), true);
    await endDesktopGame();
    assert.equal(fullscreenCalls, 0);
    assert.equal(lockCalls, 0);
  }
  console.log('PASS desktop and touch laptop keep ordinary viewport, focus and input');
}

function bothEndian(bytes, offset, value, width) {
  const fields = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
  if (width === 2) {
    fields.setUint16(offset, value, true);
    fields.setUint16(offset + width, value);
  } else {
    fields.setUint32(offset, value, true);
    fields.setUint32(offset + width, value);
  }
}

function record(name, lba, size, {directory = false, joliet = false} = {}) {
  const encoded = typeof name === 'number' ? Uint8Array.of(name) : encoder.encode(name);
  const id = joliet && typeof name !== 'number'
    ? Uint8Array.from([...encoded].flatMap(byte => [0, byte]))
    : encoded;
  const bytes = new Uint8Array(33 + id.length + (id.length % 2 === 0));
  bytes[0] = bytes.length;
  bothEndian(bytes, 2, lba, 4);
  bothEndian(bytes, 10, size, 4);
  bytes[25] = directory ? 2 : 0;
  bothEndian(bytes, 28, 1, 2);
  bytes[32] = id.length;
  bytes.set(id, 33);
  return bytes;
}

function makeIso(stride, offset, {joliet = false, sfx = false} = {}) {
  const image = new Uint8Array(40 * stride);
  const sector = lba => image.subarray(lba * stride + offset, lba * stride + offset + blockSize);
  const root = joliet ? 22 : 19;
  const subdir = 24;
  const v1 = encoder.encode('LEMB disc patch');
  const base = new Uint8Array(blockSize + 11).fill(42);
  base.set(encoder.encode('LEMB'));
  const installer = encoder.encode('synthetic SFX payload');

  function descriptor(lba, type, rootLba) {
    sector(lba).set([type, ...encoder.encode('CD001'), 1]);
    if (type === 255) return;
    if (type === 2) sector(lba).set(encoder.encode('%/E'), 88);
    bothEndian(sector(lba), 128, blockSize, 2);
    sector(lba).set(record(0, rootLba, 2 * blockSize, {directory: true}), 156);
  }
  function entries(lba, ...records) {
    let at = 0;
    for (const record of records) {
      sector(lba).set(record, at);
      at += record.length;
    }
  }

  descriptor(16, 1, 19);
  if (joliet) {
    descriptor(17, 2, root);
    entries(19, record(0, 19, 2 * blockSize, {directory: true}));
  }
  descriptor(joliet ? 18 : 17, 255);
  entries(root, record(0, root, 2 * blockSize, {directory: true}),
    record(1, root, 2 * blockSize, {directory: true}),
    record(joliet ? 'Lemmings Revolution Patch' : 'LEMMIN~1', subdir, blockSize, {directory: true, joliet}));
  // A directory can continue past the zero padding at the end of a sector.
  entries(root + 1,
    record('LEMMINGS.BOX;1', 31, base.length, {joliet}),
    record('PaTcH.BoX;1', 30, v1.length, {joliet}));
  if (sfx) entries(subdir, record('Lempatch.Exe;1', 34, installer.length, {joliet}));
  sector(30).set(v1);
  sector(31).set(base.subarray(0, blockSize));
  sector(32).set(base.subarray(blockSize));
  sector(34).set(installer);
  return {image, base, v1, installer};
}

async function synthetic() {
  for (const [name, stride, offset, options] of [
    ['2048', 2048, 0, {}],
    ['Mode 1', 2352, 16, {sfx: true}],
    ['Mode 2 Joliet', 2352, 24, {sfx: true, joliet: true}],
  ]) {
    const {image, base, v1, installer} = makeIso(stride, offset, options);
    const source = new Blob([image]);
    const reads = [];
    let extractions = 0;
    const updatedPatch = new Blob(['LEMB extracted test patch']);
    const files = await readCdImage({
      size: source.size,
      slice(start, end) {
        reads.push([start, end]);
        return source.slice(start, end);
      },
    }, async sfx => {
      extractions++;
      assert.deepEqual(new Uint8Array(await sfx.arrayBuffer()), installer);
      return updatedPatch;
    });
    assert.deepEqual(new Uint8Array(await files['lemmings.box'].arrayBuffer()), base);
    assert.deepEqual(new Uint8Array(await files['patch.box'].arrayBuffer()),
      options.sfx ? new Uint8Array(await updatedPatch.arrayBuffer()) : v1);
    assert.equal(extractions, Number(Boolean(options.sfx)));
    assert.ok(reads.every(([start, end]) => end - start < source.size));
    console.log(`PASS local ${name}, ${options.sfx ? 'SFX preferred' : 'root patch fallback'}`);
  }

  const base = new File(['LEMB synthetic base'], 'LeMmInGs.BoX');
  const patch = new File(['LEMB synthetic patch'], 'PATCH.BOX');
  const files = selectInstalledFiles([{file: base, depth: 1}, {file: patch, depth: 1}]);
  let stored;
  let persisted = false;
  const service = createGameData({
    store: {
      load: () => stored,
      save: data => { stored = data; },
      clear: () => { stored = undefined; },
    },
    storage: {
      persist: () => { persisted = true; return true; },
      persisted: () => persisted,
    },
  });
  const validated = await service.validate(files);
  assert.equal(validated.notes.length, 2);
  assert.match(validated.notes[0], /lemmings\.box.*unrecognized SHA-256/);
  assert.match(validated.notes[1], /patch\.box.*unrecognized SHA-256/);
  assert.equal(await service.save(validated), true);
  assert.equal(await service.persisted(), true);
  assert.deepEqual(await service.load(), validated);
  await service.clear();
  assert.equal(await service.load(), undefined);
  await assert.rejects(service.validate({...files, 'patch.box': new Blob(['broken'])}), /patch\.box.*LEMB/);
  assert.throws(() => selectInstalledFiles([{file: base, depth: 1}]), /patch\.box/);
  const fallback = createGameData({
    fetchFile: async path => {
      assert.equal(path, './game/lemmings.box');
      return new Response('<!doctype html>', {headers: {'Content-Type': 'text/html'}});
    },
  });
  assert.equal(await fallback.hostedData(), null);
  const playtest = createGameData({
    fetchFile: async path => new Response(files[path.slice('./game/'.length)]),
  });
  assert.deepEqual((await playtest.hostedData()).notes, validated.notes);
  console.log('PASS installed selectors, header validation, storage contract and playtest-server fallback');

  const abort = new AbortController();
  let canceled = false;
  const interrupted = createGameData({
    fetchFile: async () => new Response(new ReadableStream({
      pull(controller) { controller.enqueue(new Uint8Array(8192)); },
      cancel() { canceled = true; },
    }, {highWaterMark: 0})),
  });
  await assert.rejects(interrupted.download(() => abort.abort(), abort.signal), {name: 'AbortError'});
  assert.ok(canceled);
  console.log('PASS Cancel stops the pending stream');
  return files;
}

async function bootWasm(path) {
  const bridge = resolve(path);
  const code = await readFile(bridge, 'utf8');
  return new Promise(resolve => {
    const options = {
      noInitialRun: true,
      onRuntimeInitialized: () => resolve(options),
    };
    const boot = new Function('Module', 'require', '__dirname', '__filename', 'module', code);
    boot(options, createRequire(bridge), dirname(bridge), bridge, {exports: {}});
  });
}

async function hashBlob(blob) {
  const hash = createHash('sha256');
  for await (const bytes of blob.stream()) hash.update(bytes);
  return hash.digest('hex');
}

async function realDisc(path, wasm) {
  const file = await open(path, 'r');
  try {
    const {size} = await file.stat();
    assert.equal(size, 773944416);
    const lastSectorEnd = (207663 + Math.ceil(5447696 / blockSize)) * 2352;
    const chunkSize = 256019;
    for (const useRange of [true, false]) {
      let requests = 0;
      let bytesServed = 0;
      let canceled = false;
      let reported = 0;
      async function read(offset, length) {
        const {buffer, bytesRead} = await file.read(Buffer.allocUnsafe(length), 0, length, offset);
        assert.equal(bytesRead, length);
        bytesServed += length;
        return buffer;
      }
      const setup = createGameData({
        extractPatch: (sfx, signal) => extractDiscPatch(wasm, sfx, signal),
        async fetchFile(url, options) {
          assert.equal(url, 'https://archive.org/cors/lemmings-revolution/Lemmings%20Revolution%20%282000%29%28Psygnosis%29.iso');
          const [, low, high] = /^bytes=(\d+)-(\d+)$/.exec(options.headers.Range);
          const start = Number(low);
          const end = Number(high) + 1;
          if (++requests === 1) assert.deepEqual([start, end], [16 * 2048, 17 * 2352]);
          if (useRange) {
            return new Response(await read(start, end - start), {
              status: 206, headers: {'Content-Range': `bytes ${start}-${end - 1}/${size}`},
            });
          }
          assert.equal(requests, 1);
          let offset = 0;
          return new Response(new ReadableStream({
            async pull(controller) {
              const length = Math.min(chunkSize, size - offset);
              if (!length) return controller.close();
              controller.enqueue(await read(offset, length));
              offset += length;
            },
            cancel() { canceled = true; },
          }, {highWaterMark: 0}));
        },
      });
      const files = await setup.download(bytes => {
        assert.ok(bytes > reported);
        reported = bytes;
      });
      assert.equal(reported, bytesServed);
      assert.equal(await hashBlob(files['lemmings.box']), expectedBase);
      assert.equal(await hashBlob(files['patch.box']), expectedPatch11);
      assert.equal(wasm.FS.analyzePath('/disc-patch').exists, false);
      if (useRange) {
        assert.ok(requests > 1);
        assert.ok(bytesServed < lastSectorEnd);
        const chunks = [];
        for (let i = 0; i < Math.ceil(5233355 / blockSize); i++) {
          const {buffer, bytesRead} = await file.read(Buffer.allocUnsafe(blockSize), 0, blockSize,
            (180517 + i) * 2352 + 24);
          assert.equal(bytesRead, blockSize);
          chunks.push(buffer);
        }
        const version10 = new Blob(chunks).slice(0, 5233355);
        assert.equal(await hashBlob(version10), expectedPatch10);
        for (const patch of [version10, files['patch.box']]) {
          const installed = selectInstalledFiles([
            {file: new File([files['lemmings.box']], 'LEMMINGS.BOX'), depth: 2},
            {file: new File([patch], 'Patch.Box'), depth: 2},
          ]);
          assert.deepEqual((await setup.validate(installed)).notes, []);
        }
        console.log(`PASS installed data with 1.0 (${expectedPatch10}) and 1.1 patches`);
      } else {
        assert.equal(requests, 1);
        assert.ok(canceled);
        assert.ok(lastSectorEnd <= bytesServed && bytesServed < lastSectorEnd + chunkSize);
      }
      console.log(`PASS ${useRange ? '206 range' : '200 stream canceled'}: requests=${requests} received=${bytesServed} lastNeededSectorEnd=${lastSectorEnd} imageBytes=${size}`);
      console.log(`  lemmings.box ${expectedBase}\n  patch.box    ${expectedPatch11}`);
    }
  } finally {
    await file.close();
  }
}

const [, , wasmPath, discPath] = process.argv;
await checkMobilePage(wasmPath ? resolve(dirname(wasmPath), 'webolution.html')
  : new URL('../web/webolution.html', import.meta.url));
const sampleFiles = await synthetic();
let discPresent = false;
if (discPath) {
  try {
    await access(discPath);
    discPresent = true;
  } catch (error) {
    if (error.code !== 'ENOENT') throw error;
  }
}
if (!discPresent) {
  console.log('SKIP real archive.org disc tests: missing ' + (discPath ?? 'ISO path'));
} else {
  assert.ok(wasmPath, 'the real disc check needs the built webolution.js');
  const wasm = await bootWasm(wasmPath);
  await realDisc(discPath, wasm);
  await mountGameData(wasm.FS, sampleFiles);
  assert.deepEqual(wasm.FS.readdir('/game').sort(), ['.', '..', 'lemmings.box', 'patch.box']);
  for (const name of Object.keys(sampleFiles)) {
    assert.equal(wasm.FS.stat('/game/' + name).mode & 0o777, 0o444);
  }
  console.log('PASS mounting both boxes read-only in MEMFS before main');
}
