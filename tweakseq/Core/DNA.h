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

#ifndef __DNA_H_
#define __DNA_H_

//A 	A 					Adenine
//C 	C 					Cytosine
//G 	G 					Guanine
//T 	T 					Thymine
//U 	U 					Uracil
//R 	A or G 			puRine
//Y 	C, T or U 	pYrimidines
//K 	G, T or U 	bases which are Ketones
//M 	A or C 			bases with aMino groups
//S 	C or G 			Strong interaction
//W 	A, T or U 	Weak interaction
//B 	not A (i.e. C,G,T or U) 	       B comes after A
//D 	not C (i.e. A,G,T or U) 	       D comes after C
//H 	not G (i.e., A,C,T or U) 	       H comes after G
//V 	neither T nor U (i.e. A,C or G)  V comes after U
//N 	A C G T U 	                     Nucleic acid
//- 	gap of indeterminate length 	
	
int StandardDNAColours[26][3]= {
	{241,191,21}, //A Adenine
	{255,255,255}, //B	not A
	{247,118,11}, //C	Cytosine
	{255,255,255}, //D	not C
	{0,0,0}, //E   
	{0,0,0}, //F  
	{153,198,249}, //G	Guanine
	{255,255,255}, //H not G
	{0,0,0}, //I
	{0,0,0}, //J
	{255,255,255}, //K G, T or U 
	{0,0,0}, //L 
	{255,255,255}, //M A or C
	{255,255,255}, //N A,C,G,T, U
	{0,0,0}, //O
	{0,0,0}, //P
	{0,0,0}, //Q
	{255,255,255}, //R A or G
	{255,255,255}, //S C or G
	{200,177,139}, //T Thymine
	{35,234,165}, //U Uracil
	{255,255,255}, //V neither T nor U
	{255,255,255}, //W  A,T or U
	{0,0,0}, //X
	{255,255,255}, //Y C,T or U
	{0,0,0} // Z
};

#endif