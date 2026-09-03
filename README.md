# FinCon

FinCon is an AI-powered Finance Controller that investigates financial exceptions that remain after reconciliation. It goes beyond identifying that a transaction does not match by determining the likely root cause, gathering supporting evidence, quantifying the financial impact, assessing confidence, and recommending the safest next action.

By combining deterministic financial controls with agentic investigation, FinCon helps finance teams reduce manual exception-handling effort, prioritize financially significant issues, and resolve cases faster while preserving human oversight, explainability, and auditability.


## Problem Statement

Modern financial systems process large volumes of payments, refunds, settlements, bank transactions, and accounting records. Reconciliation helps identify discrepancies, but finding a mismatch is only the beginning.

When an exception occurs, finance teams still need to investigate **what went wrong, why it happened, which records are related, how much money is affected, and what action should be taken**. This often requires manually searching across multiple systems and validating evidence.

The challenge is to build an intelligent Finance Controller that can **automate this investigation process**, provide evidence-backed conclusions and financial impact, while escalating uncertain or sensitive cases for human review instead of making unsafe decisions.


## Solution

FinCon acts as an intelligent investigation layer on top of financial reconciliation. Instead of stopping when reconciliation identifies an exception, FinCon automatically investigates the case across related financial records.

It combines **deterministic financial analysis with agentic investigation** to gather relevant evidence, identify the probable root cause, calculate the financial impact, assess confidence, and determine the appropriate outcome.

Every investigation produces a structured, auditable result:

**Exception → Evidence → Root Cause → ₹ Impact → Confidence → Decision → Recommendation → Audit Trail**

When the evidence is insufficient or the risk is too high, FinCon does not guess. It can request more evidence, escalate the case for human review, or mark it as unresolved.


## Tech Stack

| Technology | Purpose |
|---|---|
| C++20 + CMake | Core financial processing and backend |
| MetaLlama | AI-powered exception investigation |
| Svelte + TypeScript | Dashboard and investigation UI |
| Python | Synthetic data generation |
| REST + SSE | API and real-time updates |


## Key Features

- Automated reconciliation and exception detection
- Evidence-based financial investigation
- Root-cause analysis with AI
- Financial impact and confidence assessment
- Human review for uncertain cases
- Complete investigation audit trail
- Live finance dashboard with real-time processing updates


## How It Works

Financial Data
→ Reconciliation
→ Exception Detection
→ Investigation
→ Evidence & Root Cause
→ ₹ Impact
→ Decision
→ Recommendation
→ Audit


# FinCon Architecture

FinCon follows a deterministic-first architecture where financial truth is established through controlled processing and reconciliation, while AI is used selectively for investigating exceptions.

The system processes financial data from ingestion through reconciliation, investigation, decisioning, audit, and the Finance Command Centre.

## Architecture

![FinCon Architecture](./architecture.png)

## Architecture Flow

```text
Ingestion
    ↓
Schema Inspection & Field Mapping
    ↓
Deterministic Validation
    ↓
Financial Data Store
    ↓
Bounded Processing Queue & Worker Pool
    ↓
Deterministic Reconciliation
    ↓
Findings & Correlation
    ↓
Business Incidents
    ↓
Investigation
    ↓
Evidence & Hypotheses
    ↓
Financial Impact
    ↓
Decision & Escalation
    ↓
Recommendation
    ↓
Audit Trail
    ↓
REST / SSE
    ↓
Finance Command Centre


1. Ingestion
Accepts heterogeneous financial JSON, maps supported schemas into canonical financial models, and rejects ambiguous or invalid mappings.

2. Stream Processing
Uses a bounded queue and worker pool to process financial batches with backpressure and idempotency controls.

3. Reconciliation
Applies deterministic reconciliation rules to identify financial discrepancies without relying on the LLM.

4. Incident Correlation
Groups related findings and financial entities into business-level incidents.

5. Investigation
Collects relevant evidence, generates hypotheses, calculates financial impact, and determines whether AI escalation is required.

6. Decision & Recommendation
Applies deterministic decision and escalation policies to determine whether a case can be automatically resolved, requires human review, needs more evidence, or remains unresolved.

7. Audit
Maintains an append-only investigation trail covering evidence, tools, hypotheses, decisions, recommendations, and completion.

8. State & API
Maintains thread-safe application state and exposes REST APIs and Server-Sent Events for live updates.

9. Finance Command Centre
Provides a live operational view of ingestion, processing, exceptions, investigations, financial exposure, decisions, and investigation activity.

## Validation & Accuracy

FinCon was tested against synthetic financial data with deliberately injected exception scenarios. The current prototype demonstrates the complete flow from reconciliation through investigation and decisioning.

| Area | Current Result |
|---|---:|
| Exception scenarios | 8 |
| Reconciliation findings | 16 |
| Business incidents | 8 |
| Finding → Incident correlation | 100% |
| Incident classification | ~80% |
| Deterministic reconciliation | ~70% |
| Investigation layer | ~65% |
| Zero-impact safe cases | 2 |
| Deliberately unresolvable cases | 1 |

The current evaluation covers duplicate settlements, missing records, timing issues, refund mismatches, partial settlements, fee discrepancies, and unresolvable cases.

These figures represent the current prototype evaluation and are not presented as production accuracy. The investigation layer includes LLM behavior, so its results can vary across runs.

### Exception Coverage

| Scenario | Incident Type | Financial Impact | Expected Outcome |
|---|---|---:|---|
| Duplicate Settlement | Duplicate Record | ₹34,843.21 | Human Review |
| Missing Record | Missing Record | ₹4,874.33 | Request More Evidence |
| Delayed Settlement | Timing Issue | ₹0 | Auto Resolve |
| Refund Mismatch | Refund Issue | ₹10.00 | Request More Evidence |
| Partial Settlement | Settlement Issue | ₹18,670.21 | Human Review |
| Bank Timing Shift | Timing Issue | ₹0 | Auto Resolve |
| Fee Discrepancy | Fee Issue | ₹5.00 | Request More Evidence |
| Unresolvable Settlement | Settlement Issue | ₹0.01 | Unresolved |

The evaluation deliberately includes both resolvable and uncertain cases to verify that FinCon does not force every exception into an automatic resolution.

## Scalability & Extensibility

The current agent is built around controlled tool calls and can be incrementally extended with additional financial tools, exception-specific investigation flows, and company policy integrations without redesigning the core architecture.

As investigations require more evidence, FinCon can incorporate RAG over company policies, SOPs, historical cases, and other approved knowledge sources to improve investigation accuracy while keeping financial calculations and decisions deterministic.

The architecture is designed to scale incrementally — from the current in-memory prototype to durable storage, distributed queues, additional workers, production connectors, richer tool sets, and policy/RAG services without changing the core investigation flow.

> **Current scope:** The prototype validates the complete investigation pipeline with controlled tools and synthetic exceptions; additional tools, company-specific policies, and RAG can be introduced as evidence and domain requirements grow.

## Current Scope, Limitations & Future Improvements

| Area | Current Scope | Limitation | Future Improvement |
|---|---|---|---|
| Financial Processing | Deterministic reconciliation and exception investigation | Prototype uses in-memory data | Durable production data store |
| AI Investigation | Agent with controlled financial tool calls | Limited tool set and stochastic LLM output | More tools and exception-specific investigation flows |
| Evidence | Structured financial evidence and validation | Limited to available prototype data | RAG for company policies, SOPs and historical cases |
| Decisioning | Confidence-aware decisions with human escalation | Production policy engine not connected | Company-specific policy and approval integration |
| Infrastructure | Bounded queue and configurable worker pool | Single-process prototype | Distributed queues and horizontally scalable workers |
| Integration | REST, SSE and structured investigation output | No production authentication or financial mutation execution | Production connectors, RBAC, monitoring and controlled actions |