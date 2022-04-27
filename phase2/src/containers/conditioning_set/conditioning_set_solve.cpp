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

#include <containers/conditioning_set/conditioning_set_header.h>

void conditioning_set::solve(variant_map & V, genotype_set & G) {
	tac.clock();

	//
	vector < int > A = vector < int > (n_haplotypes, 0);
	vector < int > B = vector < int > (n_haplotypes, 0);
	vector < int > C = vector < int > (n_haplotypes, 0);
	vector < int > D = vector < int > (n_haplotypes, 0);
	vector < int > R = vector < int > (n_haplotypes, 0);
	iota(A.begin(), A.end(), 0);
	random_shuffle(A.begin(), A.end());

	//
	vector < float > scoreBit = vector < float > (n_scaffold_variants, 0.0);
	for (int l = 0 ; l < n_scaffold_variants ; ++l) scoreBit[l] = log (l + 1.0);


	//PBWT backward sweep
	tac.clock();
	for (int vt = V.sizeFull()-1 ; vt >= 0 ; vt --) {
		int vc = V.vec_full[vt]->idx_common;
		int vr = V.vec_full[vt]->idx_rare;
		int vs = V.vec_full[vt]->idx_scaffold;

		if (vs >= 0) {
			bool eval = sites_pbwt_evaluation[vs];
			bool selc = sites_pbwt_selection[vs];
			if (eval) {
				int u = 0, v = 0;
				for (int h = 0 ; h < n_haplotypes ; h ++) {
					if (!Hvar.get(vs, A[h])) A[u++] = A[h];
					else B[v++] = A[h];
				}
				std::copy(B.begin(), B.begin()+v, A.begin()+u);
				for (int h = 0 ; h < n_haplotypes ; h ++) R[A[h]] = h;
			}
		} else if (vr >= 0) solveRare1(A, R, G, vr);
		vrb.progress("  * PBWT backward pass", (V.sizeFull()-vt) * 1.0 / V.sizeFull());
	}
	vrb.bullet("PBWT backward pass (" + stb.str(tac.rel_time()*1.0/1000, 2) + "s)");

	//PBWT forward sweep
	tac.clock();
	int nzvs = 0;
	for (int vt = 0 ; vt < V.sizeFull() ; vt ++) {
		int vc = V.vec_full[vt]->idx_common;
		int vr = V.vec_full[vt]->idx_rare;
		int vs = V.vec_full[vt]->idx_scaffold;

		if (vs >= 0) {
			bool eval = sites_pbwt_evaluation[vs];
			bool selc = sites_pbwt_selection[vs];
			if (eval) {
				int u = 0, v = 0, p = vs, q = vs;
				for (int h = 0 ; h < n_haplotypes ; h ++) {
					int alookup = A[h], dlookup = C[h];
					if (dlookup > p) p = dlookup;
					if (dlookup > q) q = dlookup;
					if (!Hvar.get(vs, alookup)) {
						A[u] = alookup;
						C[u] = p;
						p = 0;
						u++;
					} else {
						B[v] = alookup;
						D[v] = q;
						q = 0;
						v++;
					}
				}
				std::copy(B.begin(), B.begin()+v, A.begin()+u);
				std::copy(D.begin(), D.begin()+v, C.begin()+u);
				for (int h = 0 ; h < n_haplotypes ; h ++) R[A[h]] = h;
				nzvs = vs;
			}
		} else if (vr >= 0) solveRare2(A, C, R, G, vr, nzvs, scoreBit);
		vrb.progress("  * PBWT forward pass", vt * 1.0 / V.sizeFull());
	}
	vrb.bullet("PBWT forward pass (" + stb.str(tac.rel_time()*1.0/1000, 2) + "s)");
}

void conditioning_set::solveRare1(vector < int > & A, vector < int > & R, genotype_set & G, unsigned int vr) {
	float thresh, v, v0, v1;
	unsigned int nm = 0, nh = 0;

	vector < int > C = vector < int > (n_haplotypes, G.major_alleles[vr]?1:-1);
	vector < int > S;
	for (int g = 0 ; g < G.GRvar_genotypes[vr].size() ; g ++) {
		if (G.GRvar_genotypes[vr][g].pha) {
			C[2*G.GRvar_genotypes[vr][g].idx+0] = G.GRvar_genotypes[vr][g].al0?1:-1;
			C[2*G.GRvar_genotypes[vr][g].idx+1] = G.GRvar_genotypes[vr][g].al1?1:-1;
		} else if (G.GRvar_genotypes[vr][g].mis) {
			S.push_back(g);
			nm++;
		} else if (G.GRvar_genotypes[vr][g].het) {
			S.push_back(g);
			nh++;
		}
	}

	//PHASING FIRST PASS
	thresh = 2.5;
	while (nh && thresh > 1.0) {
		int nhOld = nh;
		int nmOld = nm;
		nh = 0; nm = 0 ;

		for (vector < int > :: iterator s = S.begin() ; s != S.end() ; ) {
			int h0 = G.GRvar_genotypes[vr][*s].idx*2+0;
			int h1 = G.GRvar_genotypes[vr][*s].idx*2+1;

			if (G.GRvar_genotypes[vr][*s].het) {
				v = 0.0;
				if (R[h0]>0) v += C[A[R[h0]-1]];
				if (R[h0]<(n_haplotypes-1)) v += C[A[R[h0]+1]];
				if (R[h1]>0) v -= C[A[R[h1]-1]];
				if (R[h1]<(n_haplotypes-1)) v -= C[A[R[h1]+1]];

				if (v > thresh) {
					C[h0] = 1.0;
					C[h1] = -1.0;
					G.GRvar_genotypes[vr][*s].pha = 1;
					G.GRvar_genotypes[vr][*s].al0 = 1;
					G.GRvar_genotypes[vr][*s].al1 = 0;
					s = S.erase(s);
				} else if (v < -thresh) {
					C[h0] = -1.0;
					C[h1] = 1.0;
					G.GRvar_genotypes[vr][*s].pha = 1;
					G.GRvar_genotypes[vr][*s].al0 = 0;
					G.GRvar_genotypes[vr][*s].al1 = 1;
					s = S.erase(s);
				} else {
					nh++;
					s++;
				}
			} else {
				if (R[h0]>0) v0 = C[A[R[h0]-1]];
				if (R[h0]<(n_haplotypes-1)) v0 += C[A[R[h0]+1]];
				if (R[h1]>0) v1 = C[A[R[h1]-1]];
				if (R[h1]<(n_haplotypes-1)) v1 += C[A[R[h1]+1]];

				if (v0 == -2 && v1 == -2) {
					C[h0] = -1.0;
					C[h1] = -1.0;
					G.GRvar_genotypes[vr][*s].pha = 1;
					G.GRvar_genotypes[vr][*s].al0 = 0;
					G.GRvar_genotypes[vr][*s].al1 = 0;
					s = S.erase(s);
				} else if (v0 == -2 && v1 == 2) {
					C[h0] = -1.0;
					C[h1] = 1.0;
					G.GRvar_genotypes[vr][*s].pha = 1;
					G.GRvar_genotypes[vr][*s].al0 = 0;
					G.GRvar_genotypes[vr][*s].al1 = 1;
					s = S.erase(s);
				} else if (v0 == 2 && v1 == -2) {
					C[h0] = 1.0;
					C[h1] = -1.0;
					G.GRvar_genotypes[vr][*s].pha = 1;
					G.GRvar_genotypes[vr][*s].al0 = 1;
					G.GRvar_genotypes[vr][*s].al1 = 0;
					s = S.erase(s);
				} else if (v0 == 2 && v1 == 2) {
					C[h0] = 1.0;
					C[h1] = 1.0;
					G.GRvar_genotypes[vr][*s].pha = 1;
					G.GRvar_genotypes[vr][*s].al0 = 1;
					G.GRvar_genotypes[vr][*s].al1 = 1;
					s = S.erase(s);
				} else {
					nm++;
					s++;
				}
			}
		}

		if (nh == nhOld) thresh -= 1.0 ;
	}
}

void conditioning_set::solveRare2(vector < int > & A, vector < int > & D, vector < int > & R, genotype_set & G, unsigned int vr, unsigned int vs, vector < float > & scoreBit) {
	float thresh, v, v0, v1;
	unsigned int nm = 0, nh = 0;

	vector < int > C = vector < int > (n_haplotypes, G.major_alleles[vr]?1:-1);
	vector < int > S;
	for (int g = 0 ; g < G.GRvar_genotypes[vr].size() ; g ++) {
		if (G.GRvar_genotypes[vr][g].pha) {
			C[2*G.GRvar_genotypes[vr][g].idx+0] = G.GRvar_genotypes[vr][g].al0?1:-1;
			C[2*G.GRvar_genotypes[vr][g].idx+1] = G.GRvar_genotypes[vr][g].al1?1:-1;
		} else if (G.GRvar_genotypes[vr][g].mis) {
			S.push_back(g);
			nm++;
		} else if (G.GRvar_genotypes[vr][g].het) {
			S.push_back(g);
			nh++;
		}
	}

	//PHASING FIRST PASS
	thresh = 2.5;
	while (nh && thresh > 1.0) {
		int nhOld = nh;
		int nmOld = nm;
		nh = 0; nm = 0 ;

		for (vector < int > :: iterator s = S.begin() ; s != S.end() ; ) {
			int h0 = G.GRvar_genotypes[vr][*s].idx*2+0;
			int h1 = G.GRvar_genotypes[vr][*s].idx*2+1;

			if (G.GRvar_genotypes[vr][*s].het) {
				v = 0.0;
				if (R[h0]>0) v += C[A[R[h0]-1]];
				if (R[h0]<(n_haplotypes-1)) v += C[A[R[h0]+1]];
				if (R[h1]>0) v -= C[A[R[h1]-1]];
				if (R[h1]<(n_haplotypes-1)) v -= C[A[R[h1]+1]];

				if (v > thresh) {
					C[h0] = 1;
					C[h1] = -1;
					G.GRvar_genotypes[vr][*s].pha = 1;
					G.GRvar_genotypes[vr][*s].al0 = 1;
					G.GRvar_genotypes[vr][*s].al1 = 0;
					s = S.erase(s);
				} else if (v < -thresh) {
					C[h0] = -1;
					C[h1] = 1;
					G.GRvar_genotypes[vr][*s].pha = 1;
					G.GRvar_genotypes[vr][*s].al0 = 0;
					G.GRvar_genotypes[vr][*s].al1 = 1;
					s = S.erase(s);
				} else {
					nh++;
					s++;
				}
			} else {
				if (R[h0]>0) v0 = C[A[R[h0]-1]];
				if (R[h0]<(n_haplotypes-1)) v0 += C[A[R[h0]+1]];
				if (R[h1]>0) v1 = C[A[R[h1]-1]];
				if (R[h1]<(n_haplotypes-1)) v1 += C[A[R[h1]+1]];

				if (v0 == -2 && v1 == -2) {
					C[h0] = -1;
					C[h1] = -1;
					G.GRvar_genotypes[vr][*s].pha = 1;
					G.GRvar_genotypes[vr][*s].al0 = 0;
					G.GRvar_genotypes[vr][*s].al1 = 0;
					s = S.erase(s);
				} else if (v0 == -2 && v1 == 2) {
					C[h0] = -1;
					C[h1] = 1;
					G.GRvar_genotypes[vr][*s].pha = 1;
					G.GRvar_genotypes[vr][*s].al0 = 0;
					G.GRvar_genotypes[vr][*s].al1 = 1;
					s = S.erase(s);
				} else if (v0 == 2 && v1 == -2) {
					C[h0] = 1;
					C[h1] = -1;
					G.GRvar_genotypes[vr][*s].pha = 1;
					G.GRvar_genotypes[vr][*s].al0 = 1;
					G.GRvar_genotypes[vr][*s].al1 = 0;
					s = S.erase(s);
				} else if (v0 == 2 && v1 == 2) {
					C[h0] = 1;
					C[h1] = 1;
					G.GRvar_genotypes[vr][*s].pha = 1;
					G.GRvar_genotypes[vr][*s].al0 = 1;
					G.GRvar_genotypes[vr][*s].al1 = 1;
					s = S.erase(s);
				} else {
					nm++;
					s++;
				}
			}
		}

		if (nh == nhOld) thresh -= 1.0 ;
	}

	//PHASING SECOND PASS
	for (vector < int > :: iterator s = S.begin() ; s != S.end() ; s++) {
		int h0 = G.GRvar_genotypes[vr][*s].idx*2+0;
		int h1 = G.GRvar_genotypes[vr][*s].idx*2+1;

		if (G.GRvar_genotypes[vr][*s].het) {
			v = 0.0;
			if (R[h0]>0) v += C[A[R[h0]-1]] * scoreBit[vs - D[R[h0]] + 1];
			if (R[h0]<(n_haplotypes-1)) v += C[A[R[h0]+1]] * scoreBit[vs - D[R[h0]+1]+1];
			if (R[h1]>0) v -= C[A[R[h1]-1]] * scoreBit[vs - D[R[h1]] + 1];
			if (R[h1]<(n_haplotypes-1)) v -= C[A[R[h1]+1]] * scoreBit[vs - D[R[h1]+1]+1];
			if (v > 0) {
				C[h0] = 1;
				C[h1] = -1;
				G.GRvar_genotypes[vr][*s].pha = 1;
				G.GRvar_genotypes[vr][*s].al0 = 1;
				G.GRvar_genotypes[vr][*s].al1 = 0;
			} else {
				C[h0] = -1;
				C[h1] = 1;
				G.GRvar_genotypes[vr][*s].pha = 1;
				G.GRvar_genotypes[vr][*s].al0 = 0;
				G.GRvar_genotypes[vr][*s].al1 = 1;
			}
		} else {
			if (R[h0]>0) v0 = C[A[R[h0]-1]] * scoreBit[vs - D[R[h0]] + 1];
			if (R[h0]<(n_haplotypes-1)) v0 += C[A[R[h0]+1]] * scoreBit[vs - D[R[h0]+1]+1];
			if (R[h1]>0) v1 = C[A[R[h1]-1]] * scoreBit[vs - D[R[h1]] + 1];
			if (R[h1]<(n_haplotypes-1)) v1 += C[A[R[h1]+1]] * scoreBit[vs - D[R[h1]+1]+1];
			G.GRvar_genotypes[vr][*s].pha = 1;
			if (v0 > 0) {
				C[h0] = 1;
				G.GRvar_genotypes[vr][*s].al0 = 1;
			} else {
				C[h0] = -1;
				G.GRvar_genotypes[vr][*s].al0 = 0;
			}
			if (v1 > 0) {
				C[h1] = 1;
				G.GRvar_genotypes[vr][*s].al1 = 1;
			} else {
				C[h1] = -1;
				G.GRvar_genotypes[vr][*s].al1 = 0;
			}
		}
	}
}