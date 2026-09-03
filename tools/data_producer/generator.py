"""Deterministic synthetic financial JSON batch generator."""
from __future__ import annotations

import random
from datetime import datetime, timedelta, timezone
from typing import Any


def generate_batch(batch_number: int, records_per_batch: int, exception_rate: float, seed: int) -> dict[str, Any]:
    if records_per_batch < 1:
        raise ValueError("records_per_batch must be positive")
    if not 0 <= exception_rate <= 1:
        raise ValueError("exception_rate must be between 0 and 1")

    randomizer = random.Random(seed + batch_number)
    base_time = datetime(2026, 9, 3, 10, 20, tzinfo=timezone.utc)
    records: list[dict[str, Any]] = []

    for index in range(records_per_batch):
        payment_id = f"PAY-{batch_number}-{index + 1}"
        order_id = f"ORD-{batch_number}-{index + 1}"
        amount = randomizer.randint(100, 250_000)
        timestamp = (base_time + timedelta(minutes=index)).isoformat().replace("+00:00", "Z")
        variant = (batch_number + index) % 3
        corrupted = randomizer.random() < exception_rate
        if corrupted:
            amount = max(1, amount + randomizer.choice([-75, 125]))

        if variant == 0:
            record = {
                "payment_id": payment_id,
                "order_id": order_id,
                "amount": amount,
                "status": "captured",
                "created_at": timestamp,
            }
        elif variant == 1:
            record = {
                "txn_id": payment_id,
                "merchant_ref": order_id,
                "gross": amount,
                "transaction_status": "success",
                "transaction_time": timestamp,
            }
        else:
            record = {
                "paymentReference": payment_id,
                "orderNumber": order_id,
                "amountPaise": amount,
                "state": "captured",
                "createdAt": timestamp,
            }
        records.append(record)

    return {
        "batchId": f"BATCH-{batch_number}",
        "records": records,
        "schemaVersion": "demo-variants-a-b-c",
    }
