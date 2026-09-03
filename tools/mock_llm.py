from http.server import BaseHTTPRequestHandler, HTTPServer
import json, re

def parse_inner(content):
    try:
        m=re.search(r'\{.*\}',content,re.S)
        return json.loads(m.group(0)) if m else {}
    except: return {}

def get_impact(incident):
    try: return int(incident.get("financial_impact",0))
    except: return 0

def severity_score(incident, evidence, hypotheses):
    fin=get_impact(incident)
    itype=incident.get("type",0) if isinstance(incident,dict) else 0
    ev=evidence if isinstance(evidence,list) else []
    hyp=hypotheses if isinstance(hypotheses,list) else []
    score=0
    if fin==0: score+=0
    elif fin<5000: score+=1
    elif fin<50000: score+=2
    elif fin<200000: score+=3
    else: score+=4
    if itype==2: score+=2
    if itype in (1,4,5): score+=1
    if not ev: score+=2
    elif len(ev)==1: score+=1
    if not hyp: score+=1
    strengths=[e.get("strength",0) for e in ev if isinstance(e,dict)]
    avg=sum(strengths)/len(strengths) if strengths else 0
    if avg>=2: score-=1
    if avg==0 and ev: score+=1
    return max(0,min(6,score)), fin, [e.get("id","") for e in ev if isinstance(e,dict) and e.get("id")][:2]

def decide_by_severity(incident, evidence, hypotheses):
    score,fin,ids=severity_score(incident,evidence,hypotheses)
    if score<=1:
        return ("AUTO_RESOLVE","HIGH",fin,"Severity low – evidence correlated, LLM confirms safe to auto-resolve",False,"",ids)
    if score==2:
        return ("AUTO_RESOLVE","MEDIUM",fin,"Severity low-medium – LLM confirms auto-resolve per policy",False,"",ids)
    if score==3:
        return ("REQUEST_MORE_EVIDENCE","MEDIUM",fin,"Severity medium – LLM requires acquirer/gateway confirmation",True,"acquirer ledger",ids)
    if score==4:
        return ("HUMAN_REVIEW","MEDIUM",fin,"Severity high – LLM escalates for human review",False,"",ids)
    return ("HUMAN_REVIEW" if score==5 else "UNRESOLVED","LOW" if score>5 else "MEDIUM",fin,
            "Severity critical – LLM unable to resolve safely" if score>5 else "Severity high – human review required",
            score>5,"additional evidence" if score>5 else "",ids)

def build_response(decision,confidence,fin,rationale,req_more,missing,ids):
    return {"choices":[{"message":{"content":json.dumps({
        "decision":decision,"confidence":confidence,"financial_impact":fin,
        "rationale":rationale,"reasoning":rationale,
        "evidence_ids":ids,"missing_evidence":missing,
        "requires_more_evidence":req_more,
        "requested_tools":[] if not req_more else [{"tool_name":"get_related_transactions","input":"incident_id"}],
        "hypotheses":[]})}}]}

class H(BaseHTTPRequestHandler):
    def do_POST(self):
        l=int(self.headers.get('Content-Length',0))
        b=self.rfile.read(l).decode() if l else "{}"
        try: j=json.loads(b)
        except: j={}
        content=""
        try:
            msgs=j.get("messages",[])
            c=msgs[0].get("content","") if msgs and isinstance(msgs[0],dict) else ""
            content=c if isinstance(c,str) else json.dumps(c)
        except: content=""
        inner=parse_inner(content)
        incident=inner.get("incident",{}) if isinstance(inner,dict) else {}
        evidence=inner.get("evidence",[]) if isinstance(inner,dict) else []
        hypotheses=inner.get("hypotheses",[]) if isinstance(inner,dict) else []
        decision,confidence,fin,rationale,req_more,missing,ids=decide_by_severity(incident,evidence,hypotheses)
        resp=build_response(decision,confidence,fin,rationale,req_more,missing,ids)
        data=json.dumps(resp).encode()
        self.send_response(200)
        self.send_header("Content-Type","application/json")
        self.send_header("Content-Length",str(len(data)))
        self.end_headers()
        self.wfile.write(data)
    def log_message(self,*a): pass

HTTPServer(("0.0.0.0",8000),H).serve_forever()
