from pathlib import Path
import pytest
import subprocess
import signal
import time

BUILD_DIR = Path(__file__).resolve().parent.parent / "build"


@pytest.fixture
def engine(tmp_path):
    input_file = tmp_path / "input.txt"
    input_file.touch()  # Ensure LogReader can open the file
    output_file = tmp_path / "output.log"

    logger = subprocess.Popen(
        [str(BUILD_DIR / "log_sink_daemon"), "--output", str(output_file)]
    )
    time.sleep(0.2)  # Wait for startup to finish
    ingest = subprocess.Popen(
        [str(BUILD_DIR / "log_processor_daemon"), "--input", str(input_file)]
    )
    time.sleep(0.2)  # Wait for startup to finish

    yield {
        "logger": logger,
        "ingest": ingest,
        "input_file": input_file,
        "output_file": output_file,
    }

    for p in (ingest, logger):
        if p.poll() is None:
            p.send_signal(signal.SIGTERM)
            try:
                p.wait(timeout=2)
            except subprocess.TimeoutExpired:
                p.kill()
