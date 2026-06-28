# concurrent-log-engine

A concurrent log processing pipeline written in modern C++20. Designed to continuously ingest a live log file, identify issue-level entries, and persist them to disk.

## What it does

Reads a log file in tail-style (waits for new content at EOF), filters lines by severity (`[WARNING]`, `[ERROR]`, `[CRITICAL]`), and writes matching entries to an output `.log` file. All three stages run concurrently in dedicated threads and communicate through bounded thread-safe queues.

## Architecture

```
LogReader ──► raw_log_queue ──► LogAnalyzer (x2) ──► filtered_issue_queue ──► LogWriter
```

| Component | Responsibility |
|---|---|
| **LogReader** | Opens input file, streams lines into `raw_log_queue`. Blocks at EOF and retries every second (behaves like `tail -f`) |
| **LogAnalyzer** | Pops from `raw_log_queue`, checks each line for `[WARNING]`/`[ERROR]`/`[CRITICAL]`, pushes matches into `filtered_issue_queue`. |
| **LogWriter** | Pops from `filtered_issue_queue`, appends each line to `output.log`, flushing after every write |
| **ThreadSafeQueue** | Bounded, templated, blocking queue. Producers block when full, consumers block when empty |

### Shutdown

`SIGINT`/`SIGTERM` are caught via `sigwait()`. On signal, all blocking `push()`/`pop()` calls unblock and return `false`, workers exit cleanly.


---
## Build

```bash
mkdir build && cd build
cmake ..
make
./log_engine_daemon
```

---

## License

This project is licensed under the [MIT License](LICENSE). Please refer to that file for the full license text and terms under which Bookit is distributed.
