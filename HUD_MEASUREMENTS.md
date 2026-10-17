# HUD measurements: messages

Base commit: f004b93. This branch adds instrumentation only.

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
