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

async function checkMobileWarning(shellPath) {
  const html = await readFile(shellPath, 'utf8');
  const notice = html.match(/<aside id="mobile-warning"[^>]*\bhidden>([\s\S]*?)<\/aside>/);
  assert.ok(notice);
  assert.match(notice[1], /<p>This game does not play well on mobile\. Play at your own discretion\.<\/p>/);
  assert.match(notice[1], /<button id="dismiss-mobile-warning" type="button">Dismiss<\/button>/);
  assert.ok(notice.index < html.indexOf('id="setup"'));
  assert.ok(notice.index < html.indexOf('id="start"'));
  const [, script] = html.match(/<script>([\s\S]*?)<\/script>/);

  const noStorage = () => { throw new Error('Storage unavailable'); };
  function visit({coarse = false, fine = false, userAgentData, getStorage = noStorage} = {}) {
    const elements = new Map();
    let gameScript;
    const queries = new Map([['(pointer: coarse)', coarse], ['(any-pointer: fine)', fine]]);
    const browser = {
      navigator: {userAgentData},
      matchMedia(query) {
        assert.ok(queries.has(query), `Unexpected pointer query ${query}`);
        return {matches: queries.get(query)};
      },
      get localStorage() { return getStorage(); },
      URL, location: {href: 'https://example.test/game/'}, crossOriginIsolated: true,
      sessionStorage: {removeItem() {}},
      document: {
        querySelector(selector) {
          if (!elements.has(selector)) elements.set(selector, {hidden: true, addEventListener() {}});
          return elements.get(selector);
        },
        createElement: () => ({}),
        head: {append(script) { gameScript = script.src; }},
      },
    };
    browser.window = browser;
    runInNewContext(script, browser, {filename: 'webolution.html'});
    assert.equal(gameScript, './webolution.js');
    assert.equal(typeof elements.get('#start').onclick, 'function');
    return {notice: elements.get('#mobile-warning'), dismiss: elements.get('#dismiss-mobile-warning')};
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
await checkMobileWarning(wasmPath ? resolve(dirname(wasmPath), 'webolution.html')
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
