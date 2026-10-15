"""Audit editable hint assessments, bindings and unresolved human review offline."""
import argparse
from collections import Counter
from pathlib import Path
from content_governance import ROOT, load, records, status, policy, read_ledger, save

GRADES={'light':6,'moderate':3,'strong':2}
RANKS={'species','genus','family','order','class','phylum','kingdom'}


def audit(root=ROOT):
    source=load(root/'resources/data/hint_education.json',{})
    species={str(s['id']):s for s in load(root/'resources/data/species.json',[])}
    profiles=load(root/'resources/data/species_profiles.json',{})
    current=records(root);baseline=load(root/'resources/compliance/review_baseline.json',{})
    events=read_ledger(root);settings=policy(root)
    errors=[];rows=[];missing=[]
    for sid,s in species.items():
        entry=source.get(sid,{})
        if entry.get('scientific_name')!=s['name']:errors.append({'id':sid,'error':'hint_identity_mismatch'})
        items=[('taxonomic',None,entry.get('taxonomic_assistance'),f'assistance:taxonomic:{sid}',None)]
        for rank,note in entry.get('etymology_hints',{}).items():
            items.append(('etymological',rank,note.get('assistance'),f'assistance:species:{sid}:etymology:{rank}',note['text']))
        m=entry.get('morphological_hint')
        if m:
            text=m.get('prefix','')+profiles.get(sid,{}).get(m.get('reference'),{}).get('text','')
            items.append(('morphological',None,m.get('assistance'),f'assistance:species:{sid}:morphological_hint',text))
        for category,rank,assessment,key,text in items:
            if not assessment:
                errors.append({'id':sid,'key':key,'error':'assistance_missing'});continue
            if assessment.get('grade') not in GRADES or not assessment.get('rationale') or not assessment.get('origin'):
                errors.append({'id':sid,'key':key,'error':'assistance_incomplete'})
            scope=assessment.get('ranks',[])
            if not isinstance(scope,list) or not scope or any(r not in RANKS for r in scope) or (rank and scope!=[rank]):
                errors.append({'id':sid,'key':key,'error':'assistance_rank_scope'})
            if assessment.get('context') not in {'safe_broad_ancestor','any_safe_options','diagnostic_contrast'}:
                errors.append({'id':sid,'key':key,'error':'assistance_context_missing'})
            record=current.get(key)
            if not record:
                errors.append({'id':sid,'key':key,'error':'assessment_not_generated'});continue
            if record['payload']['assessment'] != assessment:
                errors.append({'id':sid,'key':key,'error':'source_assessment_not_regenerated'})
            if category=='morphological' and (m.get('photo_id')!=s.get('photo',{}).get('id') or not set(scope).issubset(m.get('ranks',[]))):
                errors.append({'id':sid,'key':key,'error':'morphology_assessment_photo_or_scope'})
            review=status(key,record,baseline,events,settings)
            old={'taxonomic':6,'etymological':3,'morphological':2}[category]
            rows.append({'key':key,'species_id':sid,'scientific_name':s['name'],'category':category,
                'ranks':scope,'text':text,'grade':assessment.get('grade'),'previous_category_ceiling':old,
                'proposed_ceiling':GRADES.get(assessment.get('grade')),'classification_changed':old!=GRADES.get(assessment.get('grade')),
                'rationale':assessment.get('rationale'),'flags':assessment.get('flags',[]),'suitability':assessment.get('suitability'),
                'origin':assessment.get('origin'),'review_state':review['state'],'reviewer':review['reviewer'],
                'reviewed_at':review['reviewed_at'],'assessment_sha256':record['content_sha256'],
                'competitive_approved':review['state'] in {'approved_editorial','approved_expert'} and review['usable']})
        for category,available in [('etymological',bool(entry.get('etymology_hints'))),('morphological',bool(m))]:
            if not available:missing.append({'species_id':sid,'scientific_name':s['name'],'category':category,'reason':'No existing suitable source content; nothing invented.'})
    return {'schema_version':1,'scope':'All active editable hint content; grades are proposals, not human certification or measured difficulty.',
            'total_species':len(species),'assessments':len(rows),'changed':sum(r['classification_changed'] for r in rows),
            'by_grade':dict(Counter(r['grade'] for r in rows)), 'by_review_state':dict(Counter(r['review_state'] for r in rows)),
            'competitive_approved':sum(r['competitive_approved'] for r in rows),'inadequate':sum(r['suitability']=='inadequate' for r in rows),
            'errors':errors,'rows':rows,'missing_content':missing}


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output',type=Path,default=ROOT/'docs/AUDITORIA_ASSISTENCIA.json')
    args=parser.parse_args();report=audit();save(args.output,report)
    print({k:v for k,v in report.items() if k not in {'rows','missing_content','errors'}})
    raise SystemExit(bool(report['errors']))
