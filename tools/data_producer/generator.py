"""Deterministic synthetic financial lifecycle batch generator."""
from __future__ import annotations

import random
from datetime import datetime, timedelta, timezone
from typing import Any


def generate_batch(batch_number: int, records_per_batch: int, exception_rate: float, seed: int) -> dict[str, Any]:
    if records_per_batch < 1:
        raise ValueError("records_per_batch must be positive")
    if not 0 <= exception_rate <= 1:
        raise ValueError("exception_rate must be between 0 and 1")

    rng = random.Random(seed + batch_number)
    base_time = datetime(2026, 9, 3, 10, 0, tzinfo=timezone.utc)

    records: list[dict[str, Any]] = []
    settlements: list[dict[str, Any]] = []
    bank_transactions: list[dict[str, Any]] = []
    accounting_entries: list[dict[str, Any]] = []
    refunds: list[dict[str, Any]] = []
    fees: list[dict[str, Any]] = []

    exception_types = [
        "missing_settlement",
        "duplicate_settlement",
        "missing_bank",
        "bank_amount_mismatch",
        "missing_accounting",
        "accounting_amount_mismatch",
        "fee_discrepancy",
        "refund_mismatch",
    ]

    for idx in range(records_per_batch):
        payment_id = f"PAY-{batch_number}-{idx + 1}"
        order_id = f"ORD-{batch_number}-{idx + 1}"
        merchant_id = f"M-{(idx % 3) + 1}"
        amount = rng.randint(10000, 500000)
        ts = (base_time + timedelta(minutes=idx * 7, seconds=batch_number)).isoformat().replace("+00:00", "Z")
        settlement_ts = (base_time + timedelta(minutes=idx * 7, hours=24)).isoformat().replace("+00:00", "Z")
        bank_ts = (base_time + timedelta(minutes=idx * 7, hours=25)).isoformat().replace("+00:00", "Z")
        acct_ts = (base_time + timedelta(minutes=idx * 7, hours=26)).isoformat().replace("+00:00", "Z")
        variant = (batch_number + idx) % 3

        if variant == 0:
            rec = {"payment_id": payment_id, "order_id": order_id, "merchantId": merchant_id, "amount": amount, "status": "captured", "created_at": ts}
        elif variant == 1:
            rec = {"txn_id": payment_id, "merchant_ref": order_id, "merchantId": merchant_id, "gross": amount, "transaction_status": "success", "transaction_time": ts}
        else:
            rec = {"paymentReference": payment_id, "orderNumber": order_id, "merchantId": merchant_id, "amountPaise": amount, "state": "captured", "createdAt": ts}
        records.append(rec)

        has_refund = rng.random() < 0.12
        refund_orig = amount // 10 if has_refund else 0
        has_fee = rng.random() < 0.70
        fee_orig = amount // 50 if has_fee else 0
        net_orig = amount - refund_orig - fee_orig

        refund_id = f"R-{batch_number}-{idx + 1}"
        fee_id = f"F-{batch_number}-{idx + 1}"
        settlement_id = f"S-{batch_number}-{idx + 1}"
        bank_id = f"B-{batch_number}-{idx + 1}"
        acct_id = f"A-{batch_number}-{idx + 1}"

        corrupted = rng.random() < exception_rate
        chosen: str | None = None
        rec_refund = refund_orig
        rec_fee = fee_orig
        set_refund = refund_orig
        set_fee = fee_orig
        set_net = net_orig
        if corrupted:
            viable = ["missing_settlement", "duplicate_settlement", "missing_bank", "bank_amount_mismatch", "missing_accounting", "accounting_amount_mismatch"]
            if has_refund:
                viable.append("refund_mismatch")
            if has_fee:
                viable.append("fee_discrepancy")
            chosen = rng.choice(viable)
            if chosen == "refund_mismatch" and has_refund:
                rec_refund = refund_orig + 1000
            elif chosen == "fee_discrepancy" and has_fee:
                rec_fee = fee_orig + 500

        if has_refund:
            refunds.append({"id": refund_id, "merchantId": merchant_id, "paymentId": payment_id, "amount": rec_refund, "status": "processed", "createdAt": ts, "processedAt": ts})
        if has_fee:
            fees.append({"id": fee_id, "merchantId": merchant_id, "paymentId": payment_id, "type": "processing", "amount": rec_fee, "createdAt": ts})

        if chosen == "missing_settlement":
            continue

        s_gross = amount
        s_net = set_net
        s_refund = set_refund
        s_fee = set_fee
        if chosen == "duplicate_settlement":
            settlements.append({"id": settlement_id, "merchantId": merchant_id, "paymentIds": [payment_id], "grossAmount": s_gross, "refundAmount": s_refund, "feeAmount": s_fee, "netAmount": s_net, "status": "settled", "createdAt": ts, "settledAt": settlement_ts})
            settlements.append({"id": settlement_id + "-DUP", "merchantId": merchant_id, "paymentIds": [payment_id], "grossAmount": s_gross, "refundAmount": s_refund, "feeAmount": s_fee, "netAmount": s_net, "status": "settled", "createdAt": ts, "settledAt": settlement_ts})
        else:
            bt_amt = s_net
            if chosen == "bank_amount_mismatch":
                bt_amt = max(1, s_net + rng.choice([-75, 125, 500]))
            settlements.append({"id": settlement_id, "merchantId": merchant_id, "paymentIds": [payment_id], "grossAmount": s_gross, "refundAmount": s_refund, "feeAmount": s_fee, "netAmount": s_net, "status": "settled", "createdAt": ts, "settledAt": settlement_ts})
            if chosen == "missing_bank":
                pass
            else:
                bank_transactions.append({"id": bank_id, "merchantId": merchant_id, "settlementId": settlement_id, "amount": bt_amt, "type": "credit", "postedAt": bank_ts})

            if chosen == "missing_accounting":
                continue
            acct_amt = s_net
            if chosen == "accounting_amount_mismatch":
                acct_amt = max(1, s_net + rng.choice([-200, 300]))
            accounting_entries.append({"id": acct_id, "merchantId": merchant_id, "reference": settlement_id, "amount": acct_amt, "type": "credit", "createdAt": acct_ts})

    return {
        "batchId": f"BATCH-{batch_number}",
        "records": records,
        "settlements": settlements,
        "bankTransactions": bank_transactions,
        "accountingEntries": accounting_entries,
        "refunds": refunds,
        "fees": fees,
        "schemaVersion": "demo-variants-a-b-c",
    }
