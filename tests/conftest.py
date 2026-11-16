import pytest
import subprocess
import signal
import time


@pytest.fixture
def engine():
    logger = subprocess.Popen(["./log_sink_daemon"], cwd="build")
    time.sleep(0.2)  # Wait for startup to finish
    ingest = subprocess.Popen(["./log_processor_daemon"], cwd="build")
    time.sleep(0.2)  # Wait for startup to finish

    yield {"logger": logger, "ingest": ingest}

    for p in (ingest, logger):
        if p.poll() is None:
            p.send_signal(signal.SIGTERM)
            try:
                p.wait(timeout=2)
            except subprocess.TimeoutExpired:
                p.kill()
