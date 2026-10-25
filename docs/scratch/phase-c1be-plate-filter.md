# C1be: stage-0 mag and min are the same local, 2 when both gates are open

2026-10-25, `c1be-plate-filter-value`, Cursor round-agent
`6d55f838-6886-44a3-b714-6e4f83f29df9`. Dispatched 12:22:58 MDT. Packet:
`build/tmp/c1be-plate-filter-value/round-note.md`. REA was used by the
worker. This check used capstone only. Zero stock starts. Nothing was
rendered.

## READ

`0x470F29` sets a local to 1. The low byte of the argument is tested at
`0x470F4D`; a zero leaves the local at 1. `[this+0x3A1]` is tested at
`0x470F5C`; a zero also leaves it at 1. Both nonzero store 2 at
`0x470F5E`. The function then pushes that local with `0x10` and calls
`[vtable+0xA0]`, and pushes the same local with `0x11` and calls it
again. Nothing between those pushes skips either call.

The ten plate submits are `0x45B66A`, before this function's `push 0` /
`call 0x470F29` at `0x45B6D9`. That argument-0 call, which keeps the
local at 1, is the text-loop bracket. `0x45B706` then pushes 1 and calls
the setter again. The selected quad at `0x45B6B5` is also before the
argument-0 call.

`0x46F234` stores the immediate 1 at `[ecx+0x3A1]`. `0x45F1E8` loads
`ecx` with `0x58CD80` and calls that function. `0x47AB84` pushes 1, loads
the same `ecx`, calls `0x470F29`, and returns. From `0x441845` through the
panel call at `0x441A71` there is no direct `call 0x470F29`.

## DERIVED

MinGW `mingw32/include/d3dtypes.h`: `D3DTSS_MAGFILTER` is 16, `D3DTSS_MINFILTER`
is 17, `D3DTFG_LINEAR` and `D3DTFN_LINEAR` are 2, and both POINT values
are 1. The pushed 2 is therefore the LINEAR request. The two indirect
calls were not observed to succeed.

## Facilitator check

The setter, the plate loop, the immediate store, the `0x58CD80` caller,
the tube-exit `push 1`, and the direct-call scan are the bytes above.
Expecting the store immediate to be 0 disagrees with the `01` at
`0x46F23A`.

Narrowing: the indirect callees between `0x441845` and `0x441A71` were
not re-opened, so a completed argument-1 request is the state on entry
only when none of them calls the setter. The worker's resolved graphs
already mark several of those callees unknown. No live device byte was
read. The next draw can sample the plates with request 2 under that
condition. It does not measure the device.
