# D2b: 20 live cursor positions against the ported pick

Queue id `d2b-pick-capture`. Round 2 of 3, after a first start that never left
the main menu. One new stock start. Worker note:
`build/tmp/d2b-pick-capture/repair-note.md`. Rows:
`build/tmp/d2b-pick-capture/picks.jsonl`.

## Question

On `MM_CLIMB_002`, for 20 cursor positions off the UI widgets, does
`pickHoveredLemming` with `priority` 0 and `distanceSquared` 0 return the
lemming stored at `[screen+0x16D8]`, or none when that pointer is 0?

## Answer

19 of 20 agree. 1 disagrees. D2 part (a) does not pass.

The repair start is pid 39764. Screen id `0x004C416C` was 3. Each of the three
Down presses was sent while the foreground owner was 39764. The mod-5
highlight intersection ended at 1 candidate (`0x0274987c`). Return was sent
only after that. The loaded path was `Compiled\MM_CLIMB_002.lvl`.

All 20 rows use the live cursor sprite box. 11 have original pointer 0. 9
name an actor-list lemming. Sample 15 is the disagreement: cursor doubles
`(450.0, 288.00018310546875)`, original pointer `0x027a54c0` (index 6), port
id 2 (pointer `0x027a3720`). Both boxes overlap the cursor. Both candidates
were passed with priority 0 and distance 0, so the port kept the earlier
index. Both have state vtable `0x004b0d28`. Facing-filter bytes
`screen+0x16b1` and `+0x16b2` are 0. `hovered_button` is -1.

The priority table at `0x004C05D0` and the distance reference at `screen+0x58`
stay unsettled. This is not D2 part (b).

The first start, pid 47392, stayed on the main menu with zero highlight
candidates and did not press Return. That start is counted. It produced no
rows.

## Facilitator check (2026-10-22)

Re-read `build/tmp/d2b-pick-capture/run.log`: pid 39764, three Downs with
foreground owner 39764, `menu-index candidates: 1`, `RETURN is being sent`,
then `loaded 'Compiled\MM_CLIMB_002.lvl'`.

Recomputed every row from `picks.jsonl` without using the stored `agree` flag.
The overlap test is the landed inclusive rule (`lemming` edge against the
cursor edge shifted by 4). With every saved key equal to 0, the earliest
overlapping index is the port's result. That recomputation matches the stored
`pick_id` on all 20 rows. It matches `original_index` on 19. Sample 15
overlaps indices 2 and 6, the recomputed pick is 2, and the original index is
6. Original-none is 11. Original-some is 9.

Install hashes after the process exited match the authority bytes: exe
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
`dgVoodoo.conf`
`d639e220814d80b7956b96bad0ca708b768458bd348bfd607afbf1913598e72b`,
`SavedGame.dat`
`f7f8603928890fb13d022d828c8a602cf7a7619ab24de82c7de4ae86ec337e6a`.
No Lemmings process was running at this check. `stock_starts_consumed` was
left at 1; both pids ran, so it is 2.

Narrowing: the zero keys are the inherited D2a limit, and this row is the
case that limit cannot decide. The gate is not amended.

**Landed.** D2 part (a) is not accepted.
