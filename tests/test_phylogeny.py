"""Offline regression coverage for the explicit source-to-navigation crosswalk."""
import copy
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'scripts'))
from phylogeny import project, placement, audit
from content_governance import load, records, status, policy


class PhylogenyTests(unittest.TestCase):
    def setUp(self):
        self.species = load(ROOT / 'resources/data/species.json')
        self.taxons = load(ROOT / 'resources/data/taxonomy.json')
        self.policy = load(ROOT / 'resources/data/phylogeny.json')

    def test_all_122_placements_and_counts(self):
        result = audit(self.species, self.taxons, self.policy)
        self.assertEqual(result['errors'], [])
        self.assertEqual(result['total'], 122)
        self.assertEqual(result['domains'], {'Eukaryota':102, 'Bacteria':20})
        self.assertEqual(result['supergroups'], {'Opisthokonta':62,'Archaeplastida':20,'Amoebozoa':10,'SAR':10})
        self.assertEqual({n['name'] for n in self.taxons if not n['parent_id']}, {'Bacteria','Archaea','Eukaryota'})

    def test_projection_is_idempotent_and_lossless(self):
        projected, nodes = project(self.species, self.taxons, self.policy)
        self.assertEqual(projected, self.species)
        self.assertEqual({n['id']:n for n in nodes}, {n['id']:n for n in self.taxons})
        before = load(ROOT / 'resources/compliance/taxonomy_migration_v1.json')['records']
        replacements = {str(row['organism_id']): row
                        for row in load(ROOT / 'docs/ETAPA_1_FOTOGRAFIAS.json')['changes']}
        self.assertEqual(set(before), {str(s['id']) for s in projected})
        for s in projected:
            previous=before[str(s['id'])]
            self.assertEqual(s['name'], previous['name'])
            replacement = replacements.get(str(s['id']))
            if replacement:
                # The immutable migration snapshot predates these explicitly
                # documented photo replacements, checked by import/governance tests.
                self.assertEqual(replacement['old_photo'], previous['photo'])
                self.assertEqual(s['photo'], replacement['new_photo'])
            else:
                self.assertEqual(s['photo'], previous['photo'])
            self.assertEqual([n['id'] for n in s['source_taxonomy']], previous['taxonomy'])
            self.assertEqual(s['legacy_educational_kingdom'], previous['legacy_educational_kingdom'])

    def test_historical_terms_are_not_natural_nodes(self):
        self.assertFalse({n['name'] for n in self.taxons} & {'Monera','Protista','Protozoa','Chromista'})
        self.assertTrue(any(n['name']=='Chromista' for s in self.species for n in s['source_taxonomy']))
        self.assertTrue(all(s.get('legacy_educational_kingdom') for s in self.species))

    def test_source_rank_is_preserved_without_fabricating_kingdoms(self):
        index={n['id']:n for n in self.taxons}
        for s in self.species:
            lineage=[index[i] for i in s['taxonomy']]
            kingdom=[n['name'] for n in lineage if n['rank']=='kingdom']
            if s['domain']=='Bacteria' or s['legacy_educational_kingdom']=='Protista': self.assertEqual(kingdom, [])
            else: self.assertEqual(kingdom,[s['legacy_educational_kingdom']])
            self.assertTrue(s['classification']['source_refs'])
            self.assertTrue(s['classification']['evidence_location'])
            self.assertEqual(s['classification']['review_state'],'draft')

    def test_unknown_and_conflicting_lineages_fail_closed(self):
        with self.assertRaises(ValueError): placement([{'name':'Unknown lineage'}],self.policy)
        with self.assertRaises(ValueError): placement([{'name':'Animalia'},{'name':'Bacteria'}],self.policy)
        with self.assertRaises(ValueError): placement([{'name':'Protozoa'}],self.policy)

    def test_additional_source_lineages_do_not_need_five_kingdoms(self):
        for ancestor,domain,clade in [('Archaea','Archaea',None),('Euglenozoa','Eukaryota','Discoba')]:
            nodes=[{'id':'fixture:root','name':ancestor,'rank':'kingdom' if ancestor=='Archaea' else 'phylum','parent_id':'','source':'synthetic_fixture','source_taxon_id':''},
                   {'id':'fixture:leaf','name':'Synthetic test organism','rank':'species','parent_id':'fixture:root','source':'synthetic_fixture','source_taxon_id':''}]
            original=[{'id':9999,'name':'Synthetic test organism','taxonomy':[n['id'] for n in nodes],'photo':{}}]
            species,projected=project(original,nodes,self.policy)
            self.assertEqual(species[0]['domain'],domain)
            self.assertNotIn('educational_kingdom',species[0])
            self.assertEqual(audit(species,projected,self.policy)['errors'],[])
            if clade:self.assertIn('clade:'+clade,species[0]['taxonomy'])

    def test_cycles_and_unjustified_changes_are_reported(self):
        nodes=copy.deepcopy(self.taxons);nodes[0]['parent_id']=nodes[0]['id']
        self.assertTrue(audit(self.species,nodes,self.policy)['errors'])
        species=copy.deepcopy(self.species);species[0]['taxonomy'][1]='clade:SAR'
        self.assertTrue(audit(species,self.taxons,self.policy)['errors'])
        nodes=copy.deepcopy(self.taxons)
        next(n for n in nodes if n['id']==self.species[0]['taxonomy'][-1])['name']='Changed identity'
        self.assertIn('projection_differs_from_source_or_policy',audit(self.species,nodes,self.policy)['errors'])

    def test_scientific_source_legacy_hashes_preserved_and_new_reviews_are_drafts(self):
        baseline=load(ROOT / 'resources/compliance/review_baseline.json')['records']
        current=records(ROOT);settings=policy(ROOT)
        for s in self.species:
            key=f"topic:{s['id']}:taxonomy"
            self.assertEqual(current[key]['content_sha256'],baseline[key]['content_sha256'])
            key=f"topic:{s['id']}:phylogeny"
            review=status(key,current[key],{},[],settings)
            self.assertEqual(review['state'],'draft')
            self.assertFalse(review['usable'])
            self.assertIsNone(review['reviewer'])

    def test_changed_evidence_invalidates_independent_review(self):
        current=records(ROOT);key=f"topic:{self.species[0]['id']}:phylogeny"
        record=current[key]
        baseline={'records':{key:{'prior_state':'approved','content_sha256':record['content_sha256']}}}
        changed=copy.deepcopy(record);changed['content_sha256']='f'*64
        self.assertEqual(status(key,changed,baseline,[],policy(ROOT))['state'],'needs_review')


if __name__=='__main__': unittest.main()
