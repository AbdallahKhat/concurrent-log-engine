import subprocess
import time

proc = subprocess.Popen(["../build/log_sink_daemon"])

time.sleep(0.5)
assert proc.poll() is None, f"Process crashed with exit code {proc.returncode}"

proc.terminate()
proc.wait(timeout=2)
print("Process launched and cleanly stopped.")

