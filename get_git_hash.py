Import("env")
import subprocess

ret = subprocess.run(
    ["git", "rev-parse", "HEAD"],
    capture_output=True, text=True, cwd=env["PROJECT_DIR"]
)
commit_hash = ret.stdout.strip()
short_hash = commit_hash[-5:] if len(commit_hash) >= 5 else "?????"

env.Append(BUILD_FLAGS=[f'-DGIT_COMMIT_SHORT=\\"{short_hash}\\"'])
