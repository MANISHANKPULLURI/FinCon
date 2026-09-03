# FinCon synthetic producer

Generates deterministic financial JSON using three source schemas and posts it to the C++ ingestion API. The producer uses only the Python standard library.

Start FinCon first:

```sh
./build/fincon
```

Then run:

```sh
python tools/data_producer/producer.py \
  --batches 20 \
  --records-per-batch 50 \
  --exception-rate 0.1 \
  --delay 1.0
```

Each batch is a JSON message sent to `POST /api/ingest`. The C++ boundary maps `payment_id`, `txn_id`, and `paymentReference` (and their equivalent amount/order fields) to the canonical model. Invalid or ambiguous mappings are rejected before queueing. The queue is bounded to eight batches; producers block while it is full, providing backpressure.

Options:

- `--server-url` changes the FinCon base URL.
- `--seed` makes generated batches repeatable.
- `--records-per-batch` controls batch size.
- `--delay` controls the pause between requests.
- `--continuous` keeps generating new deterministic batches until interrupted with `Ctrl-C`.
