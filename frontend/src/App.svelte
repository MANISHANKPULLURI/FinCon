<script lang="ts">
  import { onMount } from 'svelte';
  type Investigation = {
    id: string; incidentId: string; status: string; outcome: string;
    confidence: string; confirmedImpact: number; llmEscalated: boolean;
    evidenceIds: string[]; hypothesisIds: string[]; toolCallIds: string[];
    recommendation?: { action: string; rationale: string; financialImpact: number; evidenceIds: string[]; confidence: string };
    evidence: { id: string; type: number; sourceId: string; description: string; financialImpact: number; strength: number }[];
    hypotheses: { id: string; description: string; status: number; confidenceScore: number; estimatedImpact: number; evidenceIds: string[] }[];
    toolCalls: { id: string; tool: string; input: string; status: number; result: string }[];
  };
  type Incident = { id: string; type: number; status: number; financialImpact: number; findingIds: string[]; entityIds: string[] };
  type Dashboard = {
    totalIncidents: number; totalInvestigations: number; completedInvestigations: number; pendingInvestigations: number;
    totalFinancialExposure: number; autoResolvedCases: number; humanReviewCases: number; requestMoreEvidenceCases: number; unresolvedCases: number; unknownCases: number;
    llmEscalations: number; messagesReceived: number; messagesQueued: number; queueDepth: number; messagesProcessed: number;
    recordsReceived: number; recordsProcessed: number; transactionsReceived: number; transactionsProcessed: number;
    matchedTransactions: number; exceptions: number; investigations: number;
    paymentsReceived: number; paymentsProcessed: number; activeWorkers: number; processingStatus: string; error: string
  };
  type Evaluation = { records: number; incidents: number; investigations: number; correctIncidentTypes: number; correctOutcomes: number; correctImpacts: number; correctLLMEscalations: number; passed: boolean };
  const incidentTypeNames = ['Unknown','SettlementIssue','RefundIssue','FeeIssue','BankIssue','AccountingIssue','MissingRecord','DuplicateRecord','TimingIssue'];
  const evidenceTypes = ['Payment','Settlement','Refund','Bank transaction','Accounting entry','Related transaction','Reconciliation finding','External reference'];
  const strengthNames = ['Weak','Moderate','Strong','Conclusive'];
  const toolStatusNames = ['Requested','Running','Succeeded','Failed'];
  const outcomeActionMap: Record<string,string> = { AutoResolve:'MARK_RESOLVED', HumanReview:'ESCALATE_TO_FINANCE', RequestMoreEvidence:'REQUEST_EVIDENCE', Unresolved:'ESCALATE_TO_FINANCE', Unknown:'ESCALATE_TO_FINANCE' };
  let dashboard: Dashboard = { totalIncidents:0, totalInvestigations:0, completedInvestigations:0, pendingInvestigations:0, totalFinancialExposure:0, autoResolvedCases:0, humanReviewCases:0, requestMoreEvidenceCases:0, unresolvedCases:0, unknownCases:0, llmEscalations:0, messagesReceived:0, messagesQueued:0, queueDepth:0, messagesProcessed:0, recordsReceived:0, recordsProcessed:0, transactionsReceived:0, transactionsProcessed:0, matchedTransactions:0, exceptions:0, investigations:0, paymentsReceived:0, paymentsProcessed:0, activeWorkers:1, processingStatus:'starting', error:'' };
  let incidents: Incident[] = [];
  let investigations: Investigation[] = [];
  let selected: Investigation | null = null;
  let selectedIncident: Incident | null = null;
  let audit: { id:string; investigationId:string; incidentId:string; actor:string; description:string; financialImpact:number }[] = [];
  let evaluation: Evaluation | null = null;
  let connection = 'connecting';
  let apiError = '';
  let events: {type:string; detail:string}[] = [];
  const money = (v:number) => { try { const bi = BigInt(Math.trunc(v)); const rupees = bi / 100n; const paise = bi % 100n; return `₹${rupees.toString().replace(/\B(?=(\d{3})+(?!\d))/g, ',')}.${paise.toString().padStart(2,'0')}`; } catch { return `₹${(v/100).toLocaleString('en-IN',{minimumFractionDigits:2})}`; } };
  const refresh = async () => {
    try {
      const dR = await fetch('/api/dashboard');
      if (!dR.ok) throw new Error('Dashboard unavailable');
      dashboard = await dR.json();
      const iR = await fetch('/api/incidents');
      if (iR.ok) { incidents = await iR.json(); incidents = [...incidents].sort((a,b)=> b.financialImpact - a.financialImpact); }
      const vR = await fetch('/api/investigations');
      if (vR.ok) investigations = await vR.json();
      try { const eR = await fetch('/api/evaluation'); evaluation = eR.ok ? await eR.json() : null; } catch {}
      apiError='';
    } catch (e) { apiError = e instanceof Error ? e.message : 'Unable to load Finance Controller data.'; }
    if (selected) await selectInvestigation(selected.id);
  };
  const selectInvestigation = async (id:string) => {
    const r = await fetch(`/api/investigations/${id}`);
    selected = r.ok ? await r.json() : null;
    selectedIncident = selected ? incidents.find(x=> x.id===selected!.incidentId) ?? null : null;
    const aR = await fetch(`/api/investigations/${id}/audit`);
    audit = aR.ok ? await aR.json() : [];
  };
  onMount(() => {
    refresh();
    const timer = window.setInterval(refresh, 2000);
    const source = new EventSource('/api/events');
    source.onopen = () => connection = 'live';
    source.onerror = () => { connection = 'reconnecting'; refresh(); };
    const handler = (e: MessageEvent) => {
      try { const d = JSON.parse(e.data); events = [{type:e.type, detail: d.detail ?? d.type ?? ''}, ...events].slice(0,20); connection='live'; refresh(); } catch { connection='live'; }
    };
    ['batch_received','batch_queued','batch_processing','reconciliation_completed','exception_detected','incident_created','investigation_started','tool_started','tool_completed','evidence_collected','hypothesis_generated','impact_calculated','decision_made','recommendation_created','investigation_completed','processing_completed','state_snapshot'].forEach(t=> source.addEventListener(t, handler as any));
    return () => { clearInterval(timer); source.close(); };
  });
</script>
<svelte:head><title>FinCon | Finance Controller</title></svelte:head>
<div class="shell">
  <header><div><span class="eyebrow">FINCON / CONTROL ROOM</span><h1>Finance Controller</h1><small>Turns post-reconciliation exceptions into evidence-backed, financially quantified decisions.</small></div><div class="live"><span class:active={connection==='live'}></span>SSE {connection} · {dashboard.activeWorkers} workers</div></header>
  <section class="hero"><div><span class="eyebrow">PROCESSING STATUS</span><strong>{dashboard.processingStatus.toUpperCase()}</strong><p>{dashboard.completedInvestigations} of {dashboard.totalInvestigations} investigations completed · queue {dashboard.queueDepth}</p><small>Live ingestion queue is active. New batches appear as they arrive.</small></div><div class="progress"><div style={`width:${dashboard.totalInvestigations ? dashboard.completedInvestigations/dashboard.totalInvestigations*100 : 0}%`}></div></div></section>
  {#if dashboard.error}<div class="error">Processing failed: {dashboard.error}</div>{/if}
  {#if apiError}<div class="error">{apiError} Retrying automatically.</div>{/if}
  <section class="metrics exec">
    <div class="metric exposure"><span>Financial exposure</span><b>{money(dashboard.totalFinancialExposure)}</b><small>sum of incident impacts</small></div>
    <div class="metric"><span>Transactions received</span><b>{dashboard.transactionsReceived}</b><small>ingested</small></div>
    <div class="metric"><span>Transactions processed</span><b>{dashboard.transactionsProcessed}</b><small>reconciled</small></div>
    <div class="metric"><span>Matched</span><b>{dashboard.matchedTransactions}</b><small>no exception</small></div>
    <div class="metric"><span>Exceptions</span><b>{dashboard.exceptions}</b><small>incidents</small></div>
    <div class="metric"><span>Investigations</span><b>{dashboard.investigations}</b><small>{dashboard.completedInvestigations} completed</small></div>
    <div class="metric"><span>Auto-resolved</span><b>{dashboard.autoResolvedCases}</b><small>safe</small></div>
    <div class="metric"><span>Human review</span><b>{dashboard.humanReviewCases}</b><small>escalated</small></div>
    <div class="metric"><span>Request more evidence</span><b>{dashboard.requestMoreEvidenceCases}</b><small>insufficient</small></div>
    <div class="metric"><span>Unresolved / Unknown</span><b>{dashboard.unresolvedCases + dashboard.unknownCases}</b><small>{dashboard.unresolvedCases} unresolved · {dashboard.unknownCases} unknown type</small></div>
    <div class="metric"><span>Queue depth</span><b>{dashboard.queueDepth}</b><small>bounded 32</small></div>
    <div class="metric"><span>Active workers</span><b>{dashboard.activeWorkers}</b><small>FINCON_WORKERS</small></div>
    <div class="metric"><span>LLM escalations</span><b>{dashboard.llmEscalations}</b><small>controlled</small></div>
  </section>
  <section class="pipeline">
    <div class="pipe-step"><span>INGESTION</span><b>{dashboard.messagesReceived}</b><small>received</small></div>
    <span class="arrow">→</span>
    <div class="pipe-step"><span>RECONCILIATION</span><b>{dashboard.transactionsProcessed}</b><small>reconciled</small></div>
    <span class="arrow">→</span>
    <div class="pipe-step"><span>EXCEPTION DETECTION</span><b>{dashboard.exceptions}</b><small>incidents</small></div>
    <span class="arrow">→</span>
    <div class="pipe-step"><span>INVESTIGATION</span><b>{dashboard.investigations}</b><small>{dashboard.completedInvestigations} done</small></div>
    <span class="arrow">→</span>
    <div class="pipe-step"><span>DECISION</span><b>{dashboard.autoResolvedCases + dashboard.humanReviewCases}</b><small>resolved/review</small></div>
  </section>
  {#if events.length>0}<section class="event-stream"><span class="eyebrow">LIVE EVENT STREAM</span><div class="event-row">{#each events.slice(0,8) as ev}<span class="ev"><b>{ev.type}</b> {ev.detail}</span>{/each}</div></section>{/if}
  {#if evaluation}<section class="evaluation"><div><span class="eyebrow">EVALUATION</span><h2>Control accuracy</h2></div><strong class:pass={evaluation.passed}>{evaluation.passed ? 'PASS' : 'IN PROGRESS / REVIEW'}</strong><div class="evaluation-grid"><span>Records <b>{evaluation.records}</b></span><span>Incident types <b>{evaluation.correctIncidentTypes}/{evaluation.incidents}</b></span><span>Outcomes <b>{evaluation.correctOutcomes}/{evaluation.investigations}</b></span><span>Impacts <b>{evaluation.correctImpacts}/{evaluation.investigations}</b></span><span>LLM <b>{evaluation.correctLLMEscalations}/{evaluation.investigations}</b></span></div></section>{/if}
  <main>
    <section class="queue"><div class="section-heading"><div><span class="eyebrow">EXCEPTION QUEUE</span><h2>Newest & Highest Impact</h2></div><span class="count">{incidents.length} cases</span></div>
      {#if incidents.length===0}<div class="empty">Waiting for reconciliation results...</div>{/if}
      {#each incidents as incident}
        {@const inv = investigations.find(x=> x.incidentId===incident.id)}
        <button class:selected={selected?.incidentId===incident.id} class="case" on:click={() => inv && selectInvestigation(inv.id)}>
          <span class="case-id">{incident.id}</span><span class="case-type">{incidentTypeNames[incident.type] ?? 'Unknown'}</span><strong>{money(incident.financialImpact)}</strong><span class="pill">{inv?.outcome ?? 'Pending'}</span><span class="case-status">{inv?.confidence ?? 'Unknown'}{inv?.llmEscalated ? ' · LLM' : ''}</span>
        </button>
      {/each}
    </section>
    <aside class="detail">
      {#if selected && selectedIncident}
        <div class="section-heading"><div><span class="eyebrow">INVESTIGATION WORKSPACE</span><h2>{selected.incidentId}</h2><small>{incidentTypeNames[selectedIncident.type]} · {selectedIncident.findingIds.length} findings · {selectedIncident.entityIds.length} entities</small></div><span class="pill strong">{selected.confidence}</span></div>
        <div class="facts-grid">
          <div><span>WHAT HAPPENED</span><b>{incidentTypeNames[selectedIncident.type]}</b><small>{selectedIncident.findingIds.join(', ') || '—'}</small></div>
          <div><span>WHY IT HAPPENED</span><b>{selected.hypotheses[0]?.description ?? 'Under investigation'}</b><small>{selected.hypotheses[0] ? `${selected.hypotheses[0].confidenceScore}% confidence` : 'Evidence gathering'}</small></div>
          <div><span>FINANCIAL IMPACT</span><b>{money(selected.confirmedImpact)}</b><small>confirmed</small></div>
          <div><span>CONFIDENCE</span><b>{selected.confidence}</b><small>{selected.status}</small></div>
          <div><span>OUTCOME</span><b>{selected.outcome}</b><small>{selected.outcome==='RequestMoreEvidence' ? 'Evidence insufficient' : selected.outcome==='Unresolved' ? 'Could not safely conclude' : selected.outcome==='HumanReview' ? 'Requires finance approval' : selected.outcome==='AutoResolve' ? 'Policy-approved' : 'Unknown classification'}</small></div>
          <div><span>RECOMMENDED ACTION</span><b>{selected.recommendation?.action ?? outcomeActionMap[selected.outcome] ?? '—'}</b><small>{selected.recommendation?.rationale?.slice(0,90) ?? ''}</small></div>
        </div>
        <div class="impact"><span>ACTION STATUS</span><b>{selected.status==='Completed' ? 'Completed' : selected.status==='Failed' ? 'Failed' : 'In Progress'}</b><small>{selected.llmEscalated ? 'LLM escalated · controlled tools' : 'Deterministic'}</small></div>
        {#if selected.recommendation}<div class="recommendation"><span class="eyebrow">FINANCE ACTION</span><h3>{selected.recommendation.action}</h3><p>{selected.recommendation.rationale}</p><div class="finance-action"><span>incidentId <b>{selected.incidentId}</b></span><span>incidentType <b>{incidentTypeNames[selectedIncident.type]}</b></span><span>financialImpact <b>{money(selected.recommendation.financialImpact)}</b></span><span>outcome <b>{selected.outcome}</b></span><span>confidence <b>{selected.recommendation.confidence ?? selected.confidence}</b></span><span>recommendedAction <b>{selected.recommendation.action}</b></span><span>actionStatus <b>{selected.status}</b></span><span>evidenceIds <b>{selected.recommendation.evidenceIds.join(', ') || '—'}</b></span></div></div>{/if}
        <div class="subsection"><h3>Related entities <small>{selectedIncident.entityIds.length}</small></h3><div class="chips">{#each selectedIncident.entityIds as e}<span class="chip">{e}</span>{/each}</div><h3>Findings <small>{selectedIncident.findingIds.length}</small></h3><div class="chips">{#each selectedIncident.findingIds as f}<span class="chip">{f}</span>{/each}</div></div>
        <div class="subsection"><h3>Evidence <small>{selected.evidence.length} items</small></h3>{#each selected.evidence as it}<div class="evidence"><div><b>{it.id}</b><span class="strength">{strengthNames[it.strength] ?? 'Unknown'}</span></div><span>{evidenceTypes[it.type] ?? 'Financial record'} · {it.sourceId}</span><p>{it.description}</p><strong>{money(it.financialImpact)}</strong></div>{/each}</div>
        <div class="subsection"><h3>Hypotheses <small>{selected.hypotheses.length} items</small></h3>{#each selected.hypotheses as it}<div class="hypothesis"><b>{it.id}</b><span>{it.description}</span><small>{it.confidenceScore}% confidence · {money(it.estimatedImpact)} · status {it.status}</small></div>{/each}</div>
        <div class="subsection"><h3>Controlled tool activity <small>{selected.toolCalls.length} calls</small></h3>{#each selected.toolCalls as it}<div class="tool"><b>✓ {it.tool}</b><span>{it.input}</span><strong>{toolStatusNames[it.status] ?? 'Unknown'}</strong><small>{it.result?.slice(0,80) ?? ''}</small></div>{/each}</div>
        <div class="subsection"><h3>Audit timeline</h3>{#each audit as e}<div class="audit"><b>{e.actor}</b><span>{e.description}</span><small>{e.financialImpact ? money(e.financialImpact) : ''}</small></div>{/each}</div>
      {:else}<div class="empty detail-empty">Select an exception to inspect evidence, decision, and audit trail. Live stream updates without refresh via SSE.</div>{/if}
    </aside>
  </main>
</div>
<style>
  header small{ display:block; margin-top:8px; color:#b9d1c4; font-size:.78rem; max-width:52ch; }
  .hero small{ display:block; margin-top:12px; color:#d7e64f; font:500 .68rem 'DM Mono', monospace; }
  .exec{ grid-template-columns: repeat(7, 1fr); }
  .exec .metric{ min-height:110px; }
  .exec .metric small{ display:block; margin-top:6px; color:#6a756e; font:500 .62rem 'DM Mono', monospace; }
  .pipeline{ display:flex; align-items:center; gap:10px; margin-top:14px; background:#173d32; color:#f4f5ed; padding:18px 20px; overflow-x:auto; }
  .pipe-step{ text-align:center; min-width:130px; }
  .pipe-step span{ display:block; font:500 .62rem 'DM Mono', monospace; letter-spacing:.06em; color:#b9d1c4; }
  .pipe-step b{ display:block; margin-top:6px; font-size:1.4rem; }
  .pipe-step small{ color:#d7e64f; font:500 .62rem 'DM Mono', monospace; }
  .arrow{ color:#d7e64f; font-size:1.2rem; }
  .event-stream{ margin-top:12px; background:#eef3e7; border:1px solid #d9ded7; padding:14px 18px; }
  .event-row{ display:flex; gap:8px; flex-wrap:wrap; margin-top:8px; }
  .ev{ background:#173d32; color:#d7e64f; padding:5px 8px; font:500 .62rem 'DM Mono', monospace; }
  .ev b{ color:#fff; margin-right:4px; }
  .facts-grid{ display:grid; grid-template-columns:1fr 1fr; gap:1px; background:#d9ded7; margin-top:14px; }
  .facts-grid div{ background:#f2f4ef; padding:14px 16px; }
  .facts-grid span{ display:block; font:500 .62rem 'DM Mono', monospace; color:#6a756e; letter-spacing:.06em; }
  .facts-grid b{ display:block; margin-top:6px; font-size:.9rem; }
  .facts-grid small{ display:block; margin-top:4px; color:#778079; font-size:.72rem; }
  .finance-action{ display:grid; grid-template-columns:1fr 1fr; gap:8px; margin-top:12px; font:500 .68rem 'DM Mono', monospace; }
  .finance-action span{ background:#eef3e7; padding:8px 10px; }
  .finance-action b{ display:block; color:#173d32; font-size:.72rem; margin-top:4px; word-break:break-all; }
  .chips{ display:flex; flex-wrap:wrap; gap:6px; margin:8px 0 12px; }
  .chip{ background:#173d32; color:#fff; padding:5px 8px; font:500 .62rem 'DM Mono', monospace; }
  .subsection h3 small{ float:right; color:#78837c; font:500 .62rem 'DM Mono', monospace; }
  .evidence, .hypothesis, .tool, .audit{ border-top:1px solid #dfe3dd; padding:11px 0; font-size:.72rem; }
  .evidence > div, .tool{ display:flex; justify-content:space-between; gap:10px; }
  .evidence > span, .hypothesis > span, .hypothesis > small, .tool small, .audit small{ display:block; color:#778079; margin-top:5px; }
  .evidence p, .audit span{ margin:7px 0; line-height:1.4; }
  .evidence > strong{ color:#087c57; }
  .strength{ background:#e4f1c8; padding:3px 6px; color:#45652d; font:500 .6rem 'DM Mono', monospace; text-transform:uppercase; }
  .tool strong{ color:#087c57; font:500 .62rem 'DM Mono', monospace; }
  .hypothesis b{ display:block; font:500 .68rem 'DM Mono', monospace; }
  .audit b{ color:#173d32; font:500 .68rem 'DM Mono', monospace; }
  @media (max-width:1100px){ .exec{ grid-template-columns: repeat(4,1fr);} .facts-grid{ grid-template-columns:1fr;} }
  @media (max-width:600px){ .exec{ grid-template-columns: repeat(2,1fr);} .pipeline{ gap:6px;} .finance-action{ grid-template-columns:1fr;}}
</style>
