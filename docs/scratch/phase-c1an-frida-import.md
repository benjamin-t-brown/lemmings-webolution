# C1an: this python can import frida 17.24.3

Queue `c1an-frida-import`. Offline. No stock start. Worker [Make frida importable](3fa29f47-eabc-4aaa-85dd-278b495b0e80). REA was not used. Dispatched 21:37 MDT.

## Result

`python -m pip install frida` installed only `frida` 17.24.3 for `C:\Program Files\Python313\python.exe`. `import frida` now resolves to the user site-packages. The c1am hook and probe were not edited. Nothing was spawned.

## Facilitator check

`python -c "import frida"` printed `17.24.3` from `C:\Users\bbrown\AppData\Roaming\Python\Python313\site-packages\frida\__init__.py`. `python build/tmp/c1an-frida-import/dry_check.py` exited 0 and re-ran the c1am dry check, which printed `dry-check ok`. `frida.spawn`, `frida.resume` and `frida.attach` are attributes of that module; none was called. SHA-256 of `hook.js` is `81bd44cf180f89b8a1454292fbfe9c8e6a4e8f9a7a6a078dd82c0f09a5ec98de` and of `probe.py` is `9048db1e645905a29eed9af5f3ec31ae4c1b8f9c683d1d9877f96cd519936bca`. Tasklist showed no Lemmings process. This shell's session id is 1. No source change, so the suite was not rebuilt.

Narrowing: the import works. `frida.spawn` of the exe was not run, so a suspended launch is still unproved.

## Stop

Round 2 of 3. The one start is next. The shift counts stay unread.
