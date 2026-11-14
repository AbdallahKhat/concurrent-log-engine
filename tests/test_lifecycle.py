import subprocess
import signal
import time

logger = subprocess.Popen(["../build/log_sink_daemon"])
time.sleep(0.5)  # Wait for IPC to initialize
ingest = subprocess.Popen(["../build/log_processor_daemon"])

try:
    time.sleep(2)  # Simulated run time
finally:
    # Always shut down both, even if errors occur
    for p in (ingest, logger):
        if p.poll() is None:
            p.send_signal(signal.SIGTERM)
            try:
                p.wait(timeout=2)
            except subprocess.TimeoutExpired:
                p.kill()
