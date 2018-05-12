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
 * Export functions; Interface.h
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Tong Xiao (email: xiaotong@mail.neu.edu.cn)
 *
 * $Last Modified by:
 *
 */


#ifndef _INTERFACE_H_
#define _INTERFACE_H_

namespace smt {

extern bool MERTraining(const char * reffn, const char * trainfn, const char * configfn, int ngram, int bleutype);
extern float CalculateBLEU(const char * reffn, const char * trainfn, int refNum, int ngram, int bleutype);
extern float CalculateBP(const char * reffn, const char * trainfn, int refNum, int ngram, int bleutype);
extern void UnloadMERTrainer();
extern void SetNormalizationLabel(bool label);
extern void SetUseF1Criterion(bool use);
extern void SetInternationalTokenization(bool token);

extern void LoadSMTModel(const char * arg);
extern void UnloadSMTModel();
extern void LoadConfig(const char * configfn);
extern void Decode(const char * infn, const char * outfn, const char * logfn);
extern void Decode(const char * infn, const char * outfn, const char * logfn, const char * configfn);

extern void CreateSMTService(int threadNum);
extern void StopSMTService();
extern void DecodeInService(const char * infn, const char * outfn, const char * logfn);
extern void DecodeInService(const char * infn, const char * outfn, const char * logfn, const char * configfn);


}

#endif

