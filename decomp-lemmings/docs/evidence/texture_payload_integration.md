# Installed bitmap payload inspection

`tools/lemmings_textures.cpp` composes recovered patch-first archive lookup with
[the packed bitmap decoder](texture_resource.md). It reads entries in place,
produces decoded bytes in memory and reports counts and byte totals. It writes
no game assets. `--entry` selects one archive member; `--all` selects all `.BMP`
names, deduplicated case-insensitively with patch precedence.

The installed base and patch archives contain 10,383 distinct selected BMP
names. Their decoded payloads total 326,294,744 bytes. The inspector requires a
complete BMP header and the `BM` signature after decoding. It counts raw BMPs,
packed containers, raw-copy blocks and compressed blocks. It performs no GDI,
graphics texture creation, alpha combination or rendered-image comparison.

`tools/check_installed_texture_payloads.py` independently reads archive indexes
and packed block headers to compute expected inventory, input/output byte sizes
and codec counts. It compares these with the C++ inspector. This check does not
provide a second Huffman implementation: exact decoded bytes are established
separately by the original-instruction comparisons in the decoder evidence.
Those native tests retain duplicate names across both archives and therefore
report a larger installed corpus than this patch-first inventory.

CTest runs `installed_texture_payloads` when `LEMMINGS_ARCHIVE` is configured.
To run manually:

```powershell
build/lemmings_textures.exe --archive 'C:/progs/Lemmings Revolution/lemmings.box' --patch 'C:/progs/Lemmings Revolution/patch.box' --all
```

The original texture factory bindings and GPU initialization remain static
context in `texture_resource.md`. Bitmap decoding alone does not establish
complete texture object behavior or a working renderer.
