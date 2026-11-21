// Clone addition. Browser game data lives outside the save filesystem.
const archiveImage = 'https://archive.org/cors/lemmings-revolution/' +
  'Lemmings%20Revolution%20%282000%29%28Psygnosis%29.iso';
const hashes = {
  'lemmings.box': ['74b544beee769b0354e64b25eaec06fe725d50d2df11a94270600ae03a7e6a51'],
  'patch.box': [
    'fd6025ddc5520fbf5c36314f602b9321ca1f088f391800722f85e0dd6ddecf1e',
    'a393a45e71d797fc5b646a9a235d1f64298129f378c6f4b79f47ec773d00de53',
  ],
};
const boxNames = Object.keys(hashes);
const logicalSize = 2048;
const layouts = [
  {stride: logicalSize, offset: 0},
  {stride: 2352, offset: 16},
  {stride: 2352, offset: 24},
];
const firstDescriptor = 16;
const probeStart = firstDescriptor * logicalSize;
const probeSize = (firstDescriptor + 1) * 2352 - probeStart;
const sectorsPerRead = 1024;
const ascii = new TextDecoder('ascii');
const unicode = new TextDecoder('utf-16be');

function isDescriptor(bytes) {
  return bytes[6] === 1 && ascii.decode(bytes.subarray(1, 6)) === 'CD001';
}

async function readLayout(read) {
  const probe = await read(probeStart, probeSize);
  const layout = layouts.find(({stride, offset}) =>
    isDescriptor(probe.subarray(firstDescriptor * stride + offset - probeStart)));
  if (!layout) throw new Error('This image has no ISO 9660 volume descriptor.');

  // Reuse the probe even when the next read starts partway through it.
  async function readSectors(lba, count) {
    const start = lba * layout.stride;
    const size = count * layout.stride;
    if (start < probeStart || start >= probeStart + probe.length) return read(start, size);
    const prefix = probe.subarray(start - probeStart, start - probeStart + size);
    if (prefix.length === size) return prefix;
    const bytes = new Uint8Array(size);
    bytes.set(prefix);
    bytes.set(await read(start + prefix.length, size - prefix.length), prefix.length);
    return bytes;
  }

  return async function* readExtent(lba, size) {
    for (let remaining = size; remaining > 0;) {
      const count = Math.min(sectorsPerRead, Math.ceil(remaining / logicalSize));
      const bytes = await readSectors(lba, count);
      const length = Math.min(remaining, count * logicalSize);
      if (layout.stride === logicalSize) {
        yield bytes.subarray(0, length);
      } else {
        const payload = new Uint8Array(length);
        for (let sector = 0; sector < count; sector++) {
          const start = sector * layout.stride + layout.offset;
          const end = start + Math.min(logicalSize, length - sector * logicalSize);
          payload.set(bytes.subarray(start, end), sector * logicalSize);
        }
        yield payload;
      }
      lba += count;
      remaining -= length;
    }
  };
}

function isoRecord(bytes) {
  if (bytes.length < 34 || !bytes[32] || 33 + bytes[32] > bytes.length) {
    throw new Error('The ISO image has a malformed directory record.');
  }
  const fields = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
  const lba = fields.getUint32(2, true);
  const size = fields.getUint32(10, true);
  if (lba !== fields.getUint32(6) || size !== fields.getUint32(14)) {
    throw new Error('The ISO image has inconsistent directory extents.');
  }
  return {
    lba: lba + bytes[1], size,
    directory: Boolean(bytes[25] & 2),
    unsupported: Boolean((bytes[25] & 0x80) || bytes[26] || bytes[27]),
    identifier: bytes.subarray(33, 33 + bytes[32]),
  };
}

async function volumeRoots(readExtent) {
  let primary;
  let joliet;
  // Stop at the descriptor terminator, before reading any file extents.
  for (let lba = firstDescriptor; lba < firstDescriptor + 64; lba++) {
    for await (const bytes of readExtent(lba, logicalSize)) {
      if (!isDescriptor(bytes)) throw new Error('Invalid ISO volume descriptor.');
      if (bytes[0] === 255) return [primary, joliet].filter(Boolean);
      const escape = ascii.decode(bytes.subarray(88, 91));
      const isJoliet = bytes[0] === 2 && ['%/@', '%/C', '%/E'].includes(escape);
      if (bytes[0] !== 1 && !isJoliet) continue;
      if (bytes[128] !== 0 || bytes[129] !== 8 || bytes[130] !== 8 || bytes[131] !== 0) {
        throw new Error('This ISO image uses an unsupported logical block size.');
      }
      const root = {extent: isoRecord(bytes.subarray(156, 190)), root: true, joliet: isJoliet};
      if (isJoliet) joliet = root;
      else primary = root;
    }
  }
  throw new Error('The ISO image has no volume descriptor terminator.');
}

async function* directoryEntries(readExtent, extent) {
  if (!extent.directory || extent.unsupported) throw new Error('Unsupported ISO directory layout.');
  for await (const bytes of readExtent(extent.lba, extent.size)) {
    for (let sector = 0; sector < bytes.length; sector += logicalSize) {
      const end = Math.min(sector + logicalSize, bytes.length);
      for (let offset = sector; offset < end && bytes[offset];) {
        const length = bytes[offset];
        if (offset + length > end) throw new Error('An ISO directory record crosses a sector boundary.');
        const entry = isoRecord(bytes.subarray(offset, offset + length));
        offset += length;
        // The single-byte identifiers 0 and 1 are the directory and its parent.
        if (entry.identifier.length === 1 && entry.identifier[0] < 2) continue;
        yield entry;
      }
    }
  }
}

async function findGameExtents(readExtent) {
  const pending = await volumeRoots(readExtent);
  const candidates = new Map();
  const listed = new Map();
  // Read the root and first-level directories in disc order. Both directory
  // trees on the archive.org image precede the requested file extents.
  while (pending.length) {
    pending.sort((a, b) => a.extent.lba - b.extent.lba);
    const {extent, root, joliet} = pending.shift();
    const key = extent.lba + '/' + extent.size;
    if (!listed.has(key)) {
      const entries = [];
      for await (const entry of directoryEntries(readExtent, extent)) entries.push(entry);
      listed.set(key, entries);
    }
    for (const entry of listed.get(key)) {
      if (root && entry.directory) {
        pending.push({extent: entry, root: false, joliet});
        continue;
      }
      if (entry.directory) continue;
      const name = (joliet ? unicode : ascii).decode(entry.identifier).replace(/;\d+$/, '').toLowerCase();
      const needed = root ? boxNames.includes(name) : name === 'lempatch.exe';
      if (needed && (!candidates.has(name) || (joliet && !candidates.get(name).joliet))) {
        candidates.set(name, {...entry, name, joliet});
      }
    }
  }
  const base = candidates.get('lemmings.box');
  const patch = candidates.get('lempatch.exe') ?? candidates.get('patch.box');
  if (!base) throw new Error('The ISO image is missing lemmings.box at its root.');
  if (!patch) throw new Error('The ISO image is missing LEMPATCH.EXE and patch.box.');
  return [base, patch].sort((a, b) => a.lba - b.lba);
}

async function readIso(read) {
  const readExtent = await readLayout(read);
  const files = {};
  for (const file of await findGameExtents(readExtent)) {
    if (file.unsupported) throw new Error(file.name + ' uses an unsupported ISO extent.');
    const parts = [];
    for await (const bytes of readExtent(file.lba, file.size)) parts.push(new Blob([bytes]));
    files[file.name] = new Blob(parts);
  }
  return files;
}

async function boxesFromIso(files, extractPatch, signal) {
  signal?.throwIfAborted();
  const patch = files['lempatch.exe']
    ? await extractPatch(files['lempatch.exe'], signal)
    : files['patch.box'];
  return {'lemmings.box': files['lemmings.box'], 'patch.box': patch};
}

export async function readCdImage(image, extractPatch, signal) {
  const files = await readIso(async (offset, size) => {
    signal?.throwIfAborted();
    if (offset + size > image.size) throw new Error('The ISO image ended before a required sector.');
    return new Uint8Array(await image.slice(offset, offset + size).arrayBuffer());
  });
  return boxesFromIso(files, extractPatch, signal);
}

function forwardReader(reader, report, signal) {
  let position = 0;
  let received = 0;
  let chunk = new Uint8Array();
  return async (offset, size) => {
    if (offset < position) throw new Error('The streamed ISO directory is out of order.');
    const bytes = new Uint8Array(size);
    for (let cursor = offset; cursor < offset + size;) {
      signal?.throwIfAborted();
      if (cursor >= received) {
        const {value, done} = await reader.read();
        if (done) throw new Error('The download ended before a required ISO sector.');
        chunk = value;
        received += chunk.length;
        report(received);
        continue;
      }
      const from = cursor - (received - chunk.length);
      const piece = chunk.subarray(from, from + offset + size - cursor);
      bytes.set(piece, cursor - offset);
      cursor += piece.length;
    }
    position = offset + size;
    return bytes;
  };
}

async function downloadIso(fetchFile, report, signal) {
  const requestRange = (offset, size) => fetchFile(archiveImage, {
    headers: {Range: `bytes=${offset}-${offset + size - 1}`},
    cache: 'no-store',
    signal,
  });
  const first = await requestRange(probeStart, probeSize);
  if (first.status === 200 && first.body) {
    const reader = first.body.getReader();
    try {
      return await readIso(forwardReader(reader, report, signal));
    } finally {
      // The SFX is followed by unrelated assets. Stop as soon as its final sector is read.
      try { await reader.cancel(); } catch { /* The request may already be aborted. */ }
      reader.releaseLock();
    }
  }
  let received = 0;
  return readIso(async (offset, size) => {
    const response = offset === probeStart && size === probeSize ? first : await requestRange(offset, size);
    if (response.status !== 206) {
      await response.body?.cancel();
      throw new Error('Archive.org did not return the requested ISO range (HTTP ' + response.status + ').');
    }
    const range = response.headers.get('Content-Range');
    // CORS may hide Content-Range. Always check the number of bytes received.
    const expected = `bytes ${offset}-${offset + size - 1}/`;
    if (range && !range.startsWith(expected)) {
      await response.body?.cancel();
      throw new Error('Archive.org returned an unexpected ISO range.');
    }
    const bytes = new Uint8Array(await response.arrayBuffer());
    if (bytes.length !== size) throw new Error('Archive.org returned an incomplete ISO range.');
    received += bytes.length;
    report(received);
    return bytes;
  });
}

export function selectInstalledFiles(files) {
  const sorted = [...files].sort((a, b) => a.depth - b.depth);
  const selected = {};
  for (const name of boxNames) {
    const match = sorted.find(({file}) => file.name.toLowerCase() === name);
    if (!match) throw new Error('Select a folder or file pair containing ' + name + '.');
    selected[name] = match.file;
  }
  return selected;
}

async function hasBoxMagic(file) {
  return file && ascii.decode(await file.slice(0, 4).arrayBuffer()) === 'LEMB';
}

async function validateBoxes(files, signal) {
  for (const name of boxNames) {
    signal?.throwIfAborted();
    if (!await hasBoxMagic(files[name])) throw new Error(name + ' must start with the LEMB box header.');
  }
  const notes = [];
  for (const name of boxNames) {
    signal?.throwIfAborted();
    const bytes = await files[name].arrayBuffer();
    signal?.throwIfAborted();
    const digest = await crypto.subtle.digest('SHA-256', bytes);
    const hex = Array.from(new Uint8Array(digest), byte => byte.toString(16).padStart(2, '0')).join('');
    if (!hashes[name].includes(hex)) notes.push(name + ' has a valid header and an unrecognized SHA-256.');
  }
  signal?.throwIfAborted();
  return notes;
}

export function indexedDbGameData(indexedDB) {
  async function transaction(mode, operation) {
    const db = await new Promise((resolve, reject) => {
      const request = indexedDB.open('webolution-game-data', 1);
      request.onupgradeneeded = () => request.result.createObjectStore('archives');
      request.onsuccess = () => resolve(request.result);
      request.onerror = () => reject(request.error);
    });
    try {
      return await new Promise((resolve, reject) => {
        const tx = db.transaction('archives', mode);
        const request = operation(tx.objectStore('archives'));
        tx.oncomplete = () => resolve(request.result);
        tx.onabort = () => reject(tx.error ?? new Error('Game data storage was interrupted.'));
      });
    } finally {
      db.close();
    }
  }
  return {
    load: () => transaction('readonly', store => store.get('installed')),
    save: data => transaction('readwrite', store => store.put(data, 'installed')),
    clear: () => transaction('readwrite', store => store.delete('installed')),
  };
}

export function createGameData({
  store,
  fetchFile = globalThis.fetch,
  storage = globalThis.navigator?.storage,
  extractPatch,
}) {
  async function persistence(method) {
    try {
      return (await storage?.[method]?.()) ?? null;
    } catch {
      return null;
    }
  }
  return {
    load: () => store.load(),
    clear: () => store.clear(),
    persisted: () => persistence('persisted'),
    async validate(files, signal) {
      return {files, notes: await validateBoxes(files, signal)};
    },
    async save(data) {
      const persistent = await persistence('persist');
      await store.save(data);
      return persistent;
    },
    readCd: (image, signal) => readCdImage(image, extractPatch, signal),
    async download(report, signal) {
      return boxesFromIso(await downloadIso(fetchFile, report, signal), extractPatch, signal);
    },
    async playtestData() {
      // On public hosts a missing path can be routed to an HTML index page.
      const base = await fetchFile('/game/lemmings.box');
      if (!base.ok || base.headers.get('Content-Type')?.includes('text/html')) {
        await base.body?.cancel();
        return null;
      }
      const lemmings = await base.blob();
      if (!await hasBoxMagic(lemmings)) throw new Error('Server lemmings.box has no LEMB box header.');
      const patch = await fetchFile('/game/patch.box');
      if (!patch.ok) {
        await patch.body?.cancel();
        throw new Error('Server patch.box returned HTTP ' + patch.status + '.');
      }
      const files = {'lemmings.box': lemmings, 'patch.box': await patch.blob()};
      return {files, notes: await validateBoxes(files)};
    },
  };
}

export async function extractDiscPatch({FS, ccall}, sfx, signal) {
  const bytes = new Uint8Array(await sfx.arrayBuffer());
  signal?.throwIfAborted();
  const directory = '/disc-patch';
  const installer = directory + '/LEMPATCH.EXE';
  const patch = directory + '/patch.box';
  FS.mkdir(directory);
  try {
    FS.writeFile(installer, bytes, {canOwn: true});
    const code = ccall('webolution_extract_rar20', 'number',
      ['string', 'string', 'string'], [installer, 'patch.box', patch]);
    if (code !== 0) throw new Error('Could not extract patch.box from LEMPATCH.EXE.');
    return new Blob([FS.readFile(patch)]);
  } finally {
    for (const path of [installer, patch]) {
      if (FS.analyzePath(path).exists) FS.unlink(path);
    }
    FS.rmdir(directory);
  }
}

export async function mountGameData(FS, files) {
  FS.mkdir('/game');
  for (const name of boxNames) {
    const path = '/game/' + name;
    FS.writeFile(path, new Uint8Array(await files[name].arrayBuffer()), {canOwn: true});
    FS.chmod(path, 0o444);
  }
}
