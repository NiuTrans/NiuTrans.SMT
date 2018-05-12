/* NiuTrans - SMT platform
 * Copyright (C) 2011, NEU-NLPLab (http://www.nlplab.com/). All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

/*
 * $Id:
 * !!!
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Rushan Chen
 *
 * $Last Modified by:
 *
 */


#ifndef _OURDECODER_PHRASE_STACK_H_
#define _OURDECODER_PHRASE_STACK_H_

class Decoder_Phrase_Stack : public BaseDecoder
{

public:
    Decoder_Phrase_Stack(){};
    ~Decoder_Phrase_Stack(){};

    void Init(Model * m){};
    void DecodeInput(DecodingSentence * sentence){};
    int DumpTransResult(int nbest, TransResult * result, DecodingLoger * log){return 0;};

};

#endif

