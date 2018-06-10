//
// tweakseq - provides an editor for and interface to various sequence alignment tools
//
// The MIT License (MIT)
//
// Copyright (c) 2000-2017  Michael J. Wouters, Merridee A. Wouters
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
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#ifndef __AMINO_ACIDS_H_
#define __AMINO_ACIDS_H_

// A ala alanine
// B asx asparagine or aspartic acid
// C cys cysteine
// D asp aspartic acid
// E glu glutamic acid
// F phe phenylalanine
// G gly glycine
// H his histidine
// I ile isoleucine
// J
// K lys lysine
// L leu leucine 
// M met methionine
// N asn asparagine
// O
// P pro proline
// Q gln glutamine
// R arg arginine
// S ser serine
// T thr threonine
// U 
// V val valine
// W trp tryptophan
// X
// Y tyr tyrosine
// Z glx glutamine or glutamic acid


// Physico-chemical Map
int PhysicoChemicalColours[26][3]= {
	{224,224,224}, // A ala alanine
	{255,0,128},   // B asx asparagine or aspartic acid UNDEFINED use mean
	{255,255,0},   // C cys cysteine
	{255,0,0},     // D asp aspartic acid
	{255,0,0},     // E glu glutamic acid
	{254,172,0},   // F phe phenylalanine
	{224,224,224}, // G gly glycine
	{135,206,235}, // H his histidine
	{224,224,224}, // I ile isoleucine
	{0,0,0},       // J
	{135,206,235}, // K lys lysine
	{224,224,224}, // L leu leucine 
	{255,255,0},   // M met methionine
	{255,0,255},   // N asn asparagine
	{0,0,0},       // O
	{0,255,0},     // P pro proline
	{255,0,255},   // Q gln glutamine
	{135,206,235}, // R arg arginine
	{255,0,0},     // S ser serine
	{255,0,0},     // T thr threonine
	{0,0,0},       // U 
	{224,224,224}, // V val valine
	{254,172,0},   // W trp tryptophan
	{0,0,0},       // X
	{254,172,0},   // Y tyr tyrosine
	{255,0,128},// Z glx glutamine or glutamic acid UNDEFINED use mean
};

// Taylor Map
int TaylorColours[26][3] = {
	{204,255,0}, // A ala alanine [0.8 1 0]
	{230,0,128}, // B asx asparagine or aspartic acid UNDEFINED use mean
	{255,255,0}, // C cys cysteine [1 1 0]
	{255,0,0},   // D asp aspartic acid [1 0 0] 
	{255,0,102}, // E glu glutamic acid [1 0 0.4]
	{0,255,102}, // F phe phenylalanine [0 1 0.4]
	{255,153,0}, // G gly glycine [1 0.6 0]
	{0,102,255}, // H his histidine [0 0.4 1]
	{102,255,0}, // I ile isoleucine [0.4 1 0]
	{0,0,0},     // J
	{102,0,255}, // K lys lysine [0.4 0 1]
	{51,255,0},  // L leu leucine [0.2 1 0]
	{0,255,0},   // M met methionine [0 1 0]
	{204,0,255}, // N asn asparagine [0.8 0 1]
	{0,0,0},     // O
	{255,204,0}, // P pro proline [1 0.8 0]
	{255,0,204}, // Q gln glutamine [1 0 0.8]
	{0,0,255},   // R arg arginine [0 0 1]
	{255,51,0},  // S ser serine [1 0.2 0]
	{255,102,0}, // T thr threonine [1 0.4 0]
	{0,0,0},     // U 
	{153, 255, 0}, // V val valine [0.6 1 0]
	{0,204,255},  // W trp tryptophan [0 0.8 1]
	{0,0,0},      // X
	{0,255,204},  // Y tyr tyrosine [0 1 0.8]
	{255,0,153}   // Z glx glutamine or glutamic acid UNDEFINED use mean
};

// RasMol "amino" scheme
int RasMolColours[26][3]={
	{200,200,200}, // A ala alanine
	{190,160,110}, // B asx asparagine or aspartic acid
	{230,230, 0},  // C cys cysteine
	{230,230, 10}, // D asp aspartic acid
	{230,230, 10}, // E glu glutamic acid
	{ 50, 50,170}, // F phe phenylalanine
	{235,235,235}, // G gly glycine
	{130,130,210}, // H his histidine
	{ 15,130, 15}, // I ile isoleucine
	{0,0,0},       // J
	{ 20, 90,255}, // K lys lysine
	{ 15,130, 15}, // L leu leucine 
	{230,230, 0},  // M met methionine
	{ 0,220,220},  // N asn asparagine
	{0,0,0},       // O
	{220,150,130}, // P pro proline
	{ 0,220,220},  // Q gln glutamine
	{ 20, 90,255}, // R arg arginine
	{250,150, 0},  // S ser serine
	{250,150, 0},  // T thr threonine
	{0,0,0},       // U 
	{ 15,130, 15}, // V val valine
	{180, 90,180}, // W trp tryptophan
	{0,0,0},       // X
	{ 50, 50,170}, // Y tyr tyrosine
	{190,160,110}  // Z glx glutamine or glutamic acid
};

#endif