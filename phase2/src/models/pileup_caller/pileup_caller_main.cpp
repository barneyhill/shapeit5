////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2018 Olivier Delaneau, University of Lausanne
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
////////////////////////////////////////////////////////////////////////////////
#include <models/pileup_caller/pileup_caller_header.h>

static int read_bam(void *data, bam1_t *b) {
	data_caller * aux = (data_caller*) data;
	int ret;
	while (1) {
		ret = aux->iter? sam_itr_next(aux->fp, aux->iter, b) : sam_read1(aux->fp, aux->hdr, b);
		if (ret < 0) break;
		if (b->core.flag & (BAM_FUNMAP | BAM_FSECONDARY | BAM_FQCFAIL | BAM_FDUP)) continue;
		if (b->core.flag & BAM_FPAIRED) {
			if (!(b->core.flag & BAM_FPROPER_PAIR)) continue;
			if (b->core.flag & BAM_FMUNMAP) continue;
			if ((b->core.flag & BAM_FREVERSE) == (b->core.flag & BAM_FMREVERSE)) continue;
		}
		if ((int)b->core.qual < aux->min_mapQ) continue;
		break;
    }
    return ret;
}

void pileup_caller::queryBAM(int ind, string fbam, unsigned long int & rare_het_total, unsigned long int & rare_het_pired) {
	pir d;
	data_caller DC(min_mapQ);
	map < string, pir > :: iterator itR;

	//Opening BAM file
	DC.open(fbam);

	//Loop over rare variants
	for (int r = 0 ; r < G.GRind_genotypes[ind].size() ; r ++) {

		//Is rare variant an het?
		if (G.GRind_genotypes[ind][r].het && V.vec_rare[G.GRind_genotypes[ind][r].idx]->isSNP()) {

			//Get pivots + infos
			string chr = V.vec_rare[G.GRind_genotypes[ind][r].idx]->chr;
			het l_gen = getScafHetsLeft(ind, r);
			het r_gen = getScafHetsRight(ind, r);

			//Get target infos
			het t_gen;
			t_gen.pos = V.vec_rare[G.GRind_genotypes[ind][r].idx]->bp - 1;
			t_gen.ref = V.vec_rare[G.GRind_genotypes[ind][r].idx]->ref[0];
			t_gen.alt = V.vec_rare[G.GRind_genotypes[ind][r].idx]->alt[0];

			//Get distance
			int l_dist = t_gen.distance(l_gen);
			int r_dist = t_gen.distance(r_gen);

			//Jump to region
			if (l_dist < 1500 && r_dist < 1500) DC.jump(chr, l_gen.pos - 1, r_gen.pos + 1);
			else if (l_dist < 1500) DC.jump(chr, l_gen.pos - 1, t_gen.pos + 1);
			else if (r_dist < 2000) DC.jump(chr, t_gen.pos - 1, r_gen.pos + 1);
			else break;

			//Parse reads
			R.clear();
			const bam_pileup1_t * v_plp;
			int n_plp = 0, curr_tid, curr_pos, curr_chr = DC.chr(chr);
			bam_plp_t s_plp = bam_plp_init(read_bam, (void*)&DC);
			while ((v_plp = bam_plp_auto(s_plp, &curr_tid, &curr_pos, &n_plp)) != 0) {
				if (curr_pos < DC.begin() || curr_pos >= DC.end()) continue;

				//Left
				if (curr_tid == curr_chr && curr_pos == l_gen.pos) {
					for (int read = 0 ; read < n_plp ; read ++) {
						const bam_pileup1_t * p = v_plp + read;
						if (p->is_del || p->is_refskip || p->indel == 1) continue;
						else if (bam_get_qual(p->b)[p->qpos] < min_baseQ) continue;
						else {
							char base = getBase(bam_seqi(bam_get_seq(p->b), p->qpos));
							if (base != l_gen.ref && base != l_gen.alt) continue;
							string qname = string(bam_get_qname(p->b)); d.clear();
							itR = R.insert(pair < string, pir > (qname, d)).first;
							itR->second.l_phr = (unsigned char)bam_get_qual(p->b)[p->qpos];
							itR->second.l_obs = 1;
							itR->second.l_all = (base == l_gen.alt);
						}
					}
				}

				//right
				if (curr_tid == curr_chr && curr_pos == r_gen.pos) {
					for (int read = 0 ; read < n_plp ; read ++) {
						const bam_pileup1_t * p = v_plp + read;
						if (p->is_del || p->is_refskip || p->indel == 1) continue;
						else if (bam_get_qual(p->b)[p->qpos] < min_baseQ) continue;
						else {
							char base = getBase(bam_seqi(bam_get_seq(p->b), p->qpos));
							if (base != r_gen.ref && base != r_gen.alt) continue;
							string qname = string(bam_get_qname(p->b)); d.clear();
							itR = R.insert(pair < string, pir > (qname, d)).first;
							itR->second.r_phr = (unsigned char)bam_get_qual(p->b)[p->qpos];
							itR->second.r_obs = 1;
							itR->second.r_all = (base == r_gen.alt);
						}
					}
				}

				//target
				if (curr_tid == curr_chr && curr_pos == t_gen.pos) {
					for (int read = 0 ; read < n_plp ; read ++) {
						const bam_pileup1_t * p = v_plp + read;
						if (p->is_del || p->is_refskip || p->indel == 1) continue;
						else if (bam_get_qual(p->b)[p->qpos] < min_baseQ) continue;
						else {
							char base = getBase(bam_seqi(bam_get_seq(p->b), p->qpos));
							if (base != t_gen.ref && base != t_gen.alt) continue;
							string qname = string(bam_get_qname(p->b)); d.clear();
							itR = R.insert(pair < string, pir > (qname, d)).first;
							itR->second.t_phr = (unsigned char)bam_get_qual(p->b)[p->qpos];
							itR->second.t_obs = 1;
							itR->second.t_all = (base == t_gen.alt);
						}
					}
				}
			}
			bam_plp_reset(s_plp);
			bam_plp_destroy(s_plp);

			//Phasing
			if (phaseWithPIRs(ind, r, l_gen, t_gen, r_gen)) {
				G.GRind_genotypes[ind][r].pir = 1;
				G.GRind_genotypes[ind][r].ph0 = t_gen.a0;
				rare_het_pired++;
			}
			rare_het_total++;
		}
	}

	//Closing BAM file
	DC.close();

}
