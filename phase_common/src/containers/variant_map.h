/*******************************************************************************
 * Copyright (C) 2022-2023 Olivier Delaneau
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 ******************************************************************************/

#ifndef _SNP_SET_H
#define _SNP_SET_H

#include <utils/otools.h>
#include <objects/variant.h>
#include <io/gmap_reader.h>

class variant_map {
public :
	//DATA
	vector < variant * > vec_pos;			//vector of variants ordered by position in bp
	multimap < int, variant * > map_pos;	//associative container of variant with position in bp

	//CONSTRUCTOR/DESTRUCTOR
	variant_map();
	~variant_map();

	//METHODS
	int size();
	vector < variant * > getByPos(int);
	//vector < variant * > getByRef(int, string &, string &);
	//variant * getByIndex(int);
	void push(variant *);
	void setGeneticMap(gmap_reader&);
	void setGeneticMap();
	int setCentiMorgan(vector < int > & pos_bp, vector < double > & pos_cM);
	int interpolateCentiMorgan(vector < int > & pos_bp, vector < double > & pos_cM);
	unsigned int length();
	double lengthcM();
};

#endif
