"""Post deterministic synthetic financial batches to FinCon."""
from __future__ import annotations

import argparse
import json
import sys
import time
import signal
from urllib.error import HTTPError, URLError
from urllib.request import Request, urlopen

from generator import generate_batch

_stop = False

def _handle_sigint(signum, frame):
    global _stop
    _stop = True
    print("\nShutting down producer...", file=sys.stderr, flush=True)

signal.signal(signal.SIGINT, _handle_sigint)
signal.signal(signal.SIGTERM, _handle_sigint)

def main() -> int:
    parser = argparse.ArgumentParser(description="Send synthetic financial batches to FinCon")
    parser.add_argument("--batches", type=int, default=5)
    parser.add_argument("--records-per-batch", type=int, default=10)
    parser.add_argument("--exception-rate", type=float, default=0.1)
    parser.add_argument("--seed", type=int, default=42)
    parser.add_argument("--delay", type=float, default=1.0)
    parser.add_argument("--continuous", action="store_true", help="continue generating batches until interrupted")
    parser.add_argument("--server-url", default="http://127.0.0.1:8080")
    args = parser.parse_args()

    if args.batches < 1 or args.delay < 0:
        parser.error("batches must be positive and delay must be non-negative")

    endpoint = args.server_url.rstrip("/") + "/api/ingest"
    batch_number = 1
    try:
        while not _stop and (args.continuous or batch_number <= args.batches):
            payload = generate_batch(
                batch_number,
                args.records_per_batch,
                args.exception_rate,
                args.seed,
            )
            request = Request(
                endpoint,
                data=json.dumps(payload).encode("utf-8"),
                headers={"Content-Type": "application/json"},
                method="POST",
            )
            try:
                with urlopen(request, timeout=10) as response:
                    print(response.status, response.read().decode("utf-8"), flush=True)
            except HTTPError as error:
                errBody = ""
                try: errBody = error.read().decode()[:300]
                except: pass
                print(f"batch {payload['batchId']} failed: {error} {errBody}", file=sys.stderr)
                if not args.continuous:
                    return 1
                print("Retrying in 2s...", file=sys.stderr)
                time.sleep(2)
                continue
            except URLError as error:
                print(f"batch {payload['batchId']} failed: {error}", file=sys.stderr)
                if not args.continuous:
                    return 1
                print("Retrying in 2s...", file=sys.stderr)
                time.sleep(2)
                continue
            batch_number += 1
            if _stop:
                break
            if args.continuous or batch_number <= args.batches:
                remaining = args.delay
                while remaining > 0 and not _stop:
                    step = min(0.1, remaining)
                    time.sleep(step)
                    remaining -= step
    except KeyboardInterrupt:
        print("\nInterrupted by user, shutting down.", file=sys.stderr)
    print(f"Producer stopped after {batch_number - 1} batches.", flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
