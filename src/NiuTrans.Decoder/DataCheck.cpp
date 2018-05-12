/* NiuTrans - SMT platform
 * Copyright (C) 2013, NEU-NLPLab (http://www.nlplab.com/). All rights reserved.
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
 * DataCheck; DataCheck.h
 * This header file implements module that check various
 * data files
 *
 * $Version:
 * 1.0.0
 *
 * $Created by:
 * Tong Xiao (email: xiaotong@mail.neu.edu.cn)
 *
 */

#include "DataCheck.h"

namespace smt{

bool DataChecker::CheckInputSentences(const char * fn, bool forTuning, int nref)
{
	char * line = new char[MAX_LINE_LENGTH];
	char * msg = new char[1024];
	int lineCount = 0;
	int sentCount = 0;

	FILE * f = fopen(fn, "r");
	if(f == NULL){
		fprintf(stderr, "ERROR! input file \"%s\" does not exist!\n", fn);
		exit(0);
	}

	if(forTuning){
		while(fgets(line, MAX_LINE_LENGTH - 1, f)){
			StringUtil::TrimRight(line);
			if(!forTuning ||
			   (forTuning && lineCount % (nref + 2) == 0)){
				if(!CheckSentence(line, msg)){
					fprintf(stderr, "ERROR! %s in the input sentence (id = %d)\n%s\n", msg, sentCount + 1, line);
					exit(0);
				}
				sentCount++;
			}
			lineCount++;
		}
	}

	fclose(f);
	delete[] line;
	delete[] msg;
}

bool DataChecker::CheckSentence(const char * sent, char * msg)
{
	// check whether the '#' character is contained
	const char * sentEnd = strstr(sent, "||||");
	const char * prohibitedChar = strchr(sent, '#');

        if(prohibitedChar == NULL)
                return true;

	if(sentEnd != NULL && prohibitedChar < sentEnd){
		sprintf(msg, "\'#\' is not allowed");
		return false;
	}

	return true;
}

bool DataChecker::CheckModelFiles(Configer * configer)
{
	return true;
}

}



