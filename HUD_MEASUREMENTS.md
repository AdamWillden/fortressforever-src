# HUD measurements: replicated

Base commit: f310226. This branch adds instrumentation only.

Build and deploy BOTH client and server from the same branch. Use a separate
dedicated server for CPU/network comparison so loopback does not hide traffic.
The main development branch is unchanged.

Console commands (server commands run in the server console):

    sv_ff_hud_measure 1
    cl_ff_hud_measure 1
    sv_ff_hud_measure_reset
    cl_ff_hud_measure_reset

Run a fixed duration scenario, then freeze collection before reporting:

    sv_ff_hud_measure 0
    cl_ff_hud_measure 0
    sv_ff_hud_measure_report
    cl_ff_hud_measure_report

Run idle, repeated medpack throws/regeneration, and jump-pad build/cancel/damage/
repair/destruction scenarios. Include build timer starts/cancellations and spy
sabotage. Use the same map, tick rate, rates, player count, actions and duration.
Warm up first, reset counters before each run, repeat at least five times, and
compare medians and variation. Compare total CPU time per run as well as calls;
the two methods have different call frequencies. HUD updates on the replicated
branch count accepted changed snapshots; on the message branch they count handler
entries. Neither proves a rendered frame changed. Initialisation calls count too.

Server rows time snapshot gathering/setters OR message construction/submission.
Client rows time UpdateFromPlayer OR message handlers, including early returns.
These are selected code regions, not complete engine serialization/decode costs.
Do not add nested timer_set time to buildtimer time: the latter can include it.
Mancannon also consumes buildtimer messages on the message branch (separate row).
Both variants retain their existing Paint and base panel ticks unchanged.

Payload bytes count submitted user-message bodies only (timer=6, others=2),
including repeat/duplicate sends. They exclude framing, reliability, compression,
baseline/entity traffic and retransmissions. Replicated wire bytes are deliberately
not estimated from C++ field sizes or HUD callbacks. Intermediate server snapshots
may never be transmitted, and unrelated player updates can invoke HUD callbacks.

Capture the server-to-client game connection with the same external packet capture
tool/settings for each branch. Compare total outbound bytes and packets over equal
windows; subtract a same-duration idle baseline, report raw totals too. Use remote
clients, no unrelated traffic in the filtered connection, and identical action
scripts. This measures connection-level change, not exact per-property attribution.
Also compare whole server/client frame timings using the engine profiler to account
for encode/decode work outside the custom scopes. Disable instrumentation for those
final runs. Timings with instrumentation include clock-call overhead and can be
noisy at microsecond scale; do not treat tiny differences as wins.

Check HUD correctness during every scenario, including join/respawn/class changes.
The message baseline intentionally retains historical behavior, including its
mancannon OnTick early return; this is not proof of equivalent coverage or behavior.
An incomplete implementation can appear cheaper because it misses updates.
No build or runtime measurements were performed when creating this branch.

## Listen-server network counters

The existing bindings also collect engine-channel traffic. No additional command
or external capture tool is needed. Rebuild the client for these new counters.
Start/reset AFTER connecting and after initial loading has settled:

    bind 8 "sv_ff_hud_measure 1;cl_ff_hud_measure 1;sv_ff_hud_measure_reset;cl_ff_hud_measure_reset;server_game_time"
    bind 9 "sv_ff_hud_measure 0;cl_ff_hud_measure 0;server_game_time;sv_ff_hud_measure_report;cl_ff_hud_measure_report"

The client report now adds incoming/outgoing channel bytes, incoming LOCALPLAYER,
USERMESSAGES and TOTAL packet-category bytes, and bytes per capture second. Duration
and counters freeze on disabling measurement. Each run must enable, reset, capture,
then disable/report; do not toggle measurement off/on within a run. Reset clears
all network baselines. Avoid reconnecting, changing maps or pausing within a run.

Interpretation:
- LOCALPLAYER includes all local-player entity updates, not just HUD properties.
- USERMESSAGES includes all user messages, not just the three HUD messages.
- Channel totals and packet-category totals are different engine accounting views;
  they need not sum or match. These are not physical Ethernet/UDP byte counts.
- The client samples packet history once per frame and once when stopped. Each
  valid incoming sequence is counted once. Overwritten history is reported as
  missed history, making category totals incomplete. Lost/choked entries are
  skipped; channel-byte totals are taken from cumulative engine counters.
- A missing/reset channel marks capture invalid. Zero incoming counters or zero
  selected categories are explicitly flagged rather than interpreted as free
  replication. Loopback support must be verified from your first actual report.
- Initial connection baselines and signon traffic are excluded by resetting after
  joining. Test joins separately if relevant.

Run a stationary idle capture and the same actions/duration on BOTH branches.
Keep FPS cap, tick rate, network rates, map, HUD settings and player count equal.
Compare idle and action runs separately; category differences are indicative, not
exact per-HUD attribution. Run several repeats. Copy the whole client report,
including validity/loopback/missed-history flags, alongside the CPU results.
Network sampling adds client overhead outside the timed HUD scopes. Disable it
for whole-frame performance profiling. No build/runtime verification was done.
