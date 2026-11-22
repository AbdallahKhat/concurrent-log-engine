# concurrent-log-engine

A concurrent log processing engine written in modern C++20. It runs as two cooperating daemons connected through POSIX message queue IPC.

## What it does

The processor daemon tails an input file, filters issue-level lines (`[WARNING]`, `[ERROR]`, `[CRITICAL]`), and publishes matches to a POSIX message queue. The sink daemon receives those messages and persists them to disk.

Both daemons support graceful shutdown via `SIGINT`/`SIGTERM` and use stop-aware workers for clean thread exit.

## Why two processes

- Better fault isolation: a crash in one daemon does not immediately stop the other.
- Clear separation of concerns: processor handles ingest/filtering, sink handles persistence.

## Architecture

```
[Processor Daemon]
LogReader ──► raw_log_queue ──► LogAnalyzer (x2) ──► filtered_issue_queue ──► LogIPCSender
																					 │
																					 ▼
																	  POSIX MQ: /mq_processor_to_sink
																					 │
																					 ▼
[Sink Daemon]
LogIPCReceiver ──► filtered_issue_queue ──► LogWriter
```

| Component | Responsibility |
|---|---|
| **LogReader** | Opens input file, streams lines into `raw_log_queue`. Blocks at EOF and retries every second (behaves like `tail -f`) |
| **LogAnalyzer** | Pops from `raw_log_queue`, checks each line for `[WARNING]`/`[ERROR]`/`[CRITICAL]`, pushes matches into `filtered_issue_queue`. |
| **LogIPCSender** | Pops filtered lines and sends them to POSIX message queue `/mq_processor_to_sink` |
| **LogIPCReceiver** | Receives lines from POSIX message queue and forwards them to the sink queue |
| **LogWriter** | Pops from sink queue, appends each line to the configured output file, flushing after every write |
| **ThreadSafeQueue** | Bounded, templated, blocking queue. Producers block when full, consumers block when empty |

### Shutdown

`SIGINT`/`SIGTERM` are caught via `sigwait()`. On signal, workers stop cleanly. The sink daemon unlinks the message queue on startup/shutdown to avoid stale IPC state.


---
## Build

```bash
mkdir build && cd build
cmake ..
make
```

## Run

From the repository root, start the sink daemon first, then the processor daemon in a second terminal:

```bash
# Terminal 1 (sink)
./build/log_sink_daemon --output /path/to/output.log
```

```bash
# Terminal 2 (processor)
./build/log_processor_daemon --input /path/to/input_file.txt
```

Both flags are optional (omitting them falls back to `output.log` and `input_file.txt` in the current working directory for quick testing)

### CLI Options

| Daemon | Flag | Default | Description |
|---|---|---|---|
| `log_processor_daemon` | `-i, --input <path>` | `input_file.txt` | Input log file to monitor (must exist and be readable) |
| `log_sink_daemon` | `-o, --output <path>` | `output.log` | Destination log file (parent directory must exist and be writable) |
| Both | `-h, --help` | — | Print usage information and exit |

Operational note: if `log_sink_daemon` is restarted, also restart `log_processor_daemon` so both processes reattach to the same message queue instance.

---

## License

This project is licensed under the [MIT License](LICENSE).
