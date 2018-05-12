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
 * phrase extraction and scorer; get_parameter.cpp
 *
 * $Version:
 * 1.1.0
 *
 * $Created by:
 * Qiang Li (email: liqiangneu@gmail.com)
 *
 * $Last Modified by:
 * 2012/09/19 add "-addsent" parameter for phrase extractor
 * 2012/09/17 add namespace parameter_controller
 * 2011/11/15 add this file
 *
 */

#include "parameter_controller.h"
using namespace parameter_controller;

void GetParameter::getParamsOfGeneLexiWei()
{
        cerr<<"####################################################################\n"
            <<"#   Generate Lexical Table (version 1.1.0)      --www.nlplab.com   #\n"
            <<"####################################################################\n"
            <<"[USAGE]\n"
            <<"          NiuTrans.PhraseExtractor     --LEX      [OPTIONS]\n"
            <<"[OPTION]\n"
            <<"    -src    :  Source  File.\n"
            <<"    -tgt    :  Target  File.\n"
            <<"    -aln    :  Aligned File.\n"
            <<"    -out    :  Output  File.\n"
#ifdef WIN32
            <<"    -sort   :  Cygwin Sort Dir.                   [optional]\n"
            <<"                 Default is \"c:\\cygwin\\bin\\sort.exe\".\n"
#endif
            <<"    -temp   :  Sort Temp Dir.                     [optional]\n"
            <<"                 Default is system temp dir.\n"
            <<"    -stem   :  Stemming File.                     [optional]\n"
            <<"    -addone :  Add One Smoothing.                 [optional]\n"
            <<"    -nthread:  Multi Thread.                      [optional]\n"
            <<"                 Default value is \"0\".\n"
            <<"[EXAMPLE]\n"
            <<"    >> Generate Lexical Weighting\n"
#ifdef WIN32
//            <<"           NiuTrans.PhraseExtractor --LEX -src -tgt -aln -out\n"
//            <<"                                    [-sort] [-temp] [-stem] [-addone]\n"
            <<"           NiuTrans.PhraseExtractor    --LEX     [-src FILE]\n"
            <<"                                                 [-tgt FILE]\n"
			<<"                                                 [-aln FILE]\n"
			<<"                                                 [-out FILE]\n"
#else
//            <<"           NiuTrans.PhraseExtractor --LEX -src -tgt -aln -out\n" 
//            <<"                                    [-temp] [-stem] [-addone]\n"
            <<"           NiuTrans.PhraseExtractor    --LEX     [-src FILE]\n"
			<<"                                                 [-tgt FILE]\n"
			<<"                                                 [-aln FILE]\n"
			<<"                                                 [-out FILE]\n"
#endif
            <<flush;
        return;
}

void GetParameter::getParamsOfExtractPhrasePairs()
{
        cerr<<"####################################################################\n"
            <<"#   Extract Phrase Table ( version 1.1.0 )    --www.nlplab.com     #\n"
            <<"####################################################################\n"
            <<"[USAGE]\n"
            <<"          NiuTrans.PhraseExtractor        --EXTP           [OPTIONS]\n"
            <<"[OPTION]\n"
            <<"              -src  :  Source  File.\n"
            <<"              -tgt  :  Target  File.\n"
            <<"              -aln  :  Aligned File.\n"
            <<"              -out  :  Output  File.\n"
            <<"           -srclen  :  Max Source Phrase Size.            [optional]\n"
            <<"                         For phrase-based, default is 3.\n"
            <<"           -tgtlen  :  Max Target Phrase Size.            [optional]\n"
            <<"                         For phrase-based, default is 5.\n"
            <<"             -null  :  Null Function.                     [optional]\n"
            <<"                         If phrase table contents NULL pairs or not.\n"
            <<"                         Default value is 1.\n"
			<<"   -maxSrcNulExtNum :  Default value is \"-srclen\".        [optional]\n"
			<<"   -maxTgtNulExtNum :  Default value is \"-tgtlen\".        [optional]\n"
            <<"           -allsent :  Extract sentences from bilingual   [optional]\n"
            <<"                       corpus with not respect to word-alignment.\n"
            <<"                         Default value is 0.\n"
			<<"            -method :  \"heuristic\" or \"compose\"           [optional]\n"
			<<"                         Default value is \"heuristic\".\n"
			<<"           -compose :  Composing-based extract.           [optional]\n"
			<<"                         Value range is Integer, default value is 2.\n"
            <<"[EXAMPLE]\n"
            <<"    >> Extract Phrase Pairs\n"
            <<"            NiuTrans.PhraseExtractor      --EXTP         [-src FILE]\n"
            <<"                                                         [-tgt FILE]\n"
            <<"                                                         [-aln FILE]\n"
            <<"                                                         [-out FILE]\n"
            <<flush;
        return;
}

void GetParameter::getParamsOfExtractHieroRules()
{
        cerr<<"####################################################################\n"
            <<"#  Extract Hierarchy Rules (version 1.1.0)       --www.nlplab.com  #\n"
            <<"####################################################################\n"
            <<"[USAGE]\n"
            <<"        NiuTrans.PhraseExtractor       --EXTH               [OPTIONS]\n"
            <<"[OPTION]\n"
            <<"   -src                  :  Inputted Source  File.\n"
            <<"   -tgt                  :  Inputted Target  File.\n"
            <<"   -aln                  :  Inputted Aligned File.\n"
            <<"   -out                  :  Outputted File.\n"
            <<"   -srclen               :  Max Sub Source Phrase Size.     [optional]\n"
            <<"                              Default value is 10.\n"
            <<"   -tgtlen               :  Max Sub Target Phrase Size.     [optional]\n"
            <<"                              Default value is 10.\n"
            <<"   -maxSrcI              :  Max Source Init rule length.    [optional]\n"
            <<"                              Default value is 3.\n"
            <<"   -maxTgtI              :  Max Target Init rule length.    [optional]\n"
            <<"                              Default value is 5.\n"
            <<"   -maxSrcH              :  Max Source Hiero rule length.   [optional]\n"
            <<"                              Default value is 5.\n"
            <<"   -maxTgtH              :  Max Target Hiero rule leghth.   [optioanl]\n"
            <<"                              Default value is \"-tgtlen\".\n"
            <<"   -maxNulExtSrcInitNum  :  Default value is \"-srclen\".     [optional]\n"
            <<"   -maxNulExtTgtInitNum  :  Default value is \"-tgtlen\".     [optional]\n"
            <<"   -maxNulExtSrcHieroNum :  Default value is \"-srclen\".     [optional]\n"
            <<"   -maxNulExtTgtHieroNum :  Default value is \"-tgtlen\".     [optional]\n"
            <<"   -minSrcSubPhrase      :  Default value is 1.             [optional]\n"
            <<"   -minTgtSubPhrase      :  Default value is 1.             [optional]\n"
            <<"   -minLexiNum           :  Default value is 1.             [optional]\n"
            <<"   -maxnonterm           :  Default value is 2.             [optional]\n"
            <<"   -alignedLexiReq       :  Default value is 1(true).       [optional]\n"
            <<"   -srcNonTermAdjacent   :  Default value is 0(false).      [optional]\n"
            <<"   -tgtNonTermAdjacent   :  Default value is 1(true).       [optional]\n"
            <<"   -duplicateHieroRule   :  Default value is 0(false).      [optional]\n"
            <<"   -unalignedEdgeInit    :  Default value is 1(true).       [optional]\n"
            <<"   -unalignedEdgeHiero   :  Default value is 0(false).      [optional]\n"
            <<"   -headnonterm          :  Default value is 1(true).       [optional]\n" 
            <<"   -null                 :  Default value is 1(true).       [optional]\n"
            <<"\n"
            <<"   -srcParseTreePath     :  Default is not assigned.        [optional]\n"
            <<"   -tgtParseTreePath     :  Default is not assigned.        [optional]\n"
            <<"[EXAMPLE]\n"
            <<"    >> Extract Hierarchy Rules\n"
//          <<"           NiuTrans.PhraseExtractor --EXTH -src FILE -tgt FILE -aln FILE -out FILE\n" 
            <<"           NiuTrans.PhraseExtractor        --EXTH          [-src FILE]\n"
			<<"                                                           [-tgt FILE]\n"
			<<"                                                           [-aln FILE]\n"
			<<"                                                           [-out FILE]\n"
            <<flush;
        return;
}

void GetParameter::getParamsOfScore()
{
        cerr<<"####################################################################\n"
            <<"# Calculate Extracted Table score (version 2.0)   --www.nlplab.com #\n"
            <<"####################################################################\n"
            <<"[USAGE]\n"
            <<"            NiuTrans.PhraseExtractor    --SCORE     [OPTIONS]\n"
            <<"[OPTION]\n"
            <<"    -tab         :  Extracted Table.\n"
            <<"    -tabinv      :  Extracted Table Inverse.\n"
            <<"    -ls2d        :  Lex File s2d.\n"
            <<"    -ld2s        :  Lex File d2s.\n"
            <<"    -out         :  Output  File.\n"
#ifdef WIN32
            <<"    -sort        :  Cygwin Sort Dir.                     [optional]\n"
            <<"                      Default is \"c:\\cygwin\\bin\\sort.exe\".\n"
#endif
            <<"    -temp        :  Sort Temp Dir.                       [optional]\n"
            <<"                      Default is system temp dir.\n"
            <<"    -cutoffInit  :  Cut Off Initial Phrase.              [optional]\n"
            <<"                      Default is 0.\n"
            <<"    -cutoffHiero :  Cut Off Hiero Rule.                  [optional]\n"
            <<"                      Default is 1.\n"
            <<"    -printAlign  :  Print Alignment in the Phrase Table. [optional]\n"
            <<"                      Default is 0(false).\n"
            <<"    -printFreq   :  Pring Frequency of Each Phrase Pair. [optional]\n"
            <<"                      Default is 0(false).\n"
            <<"    -method      :  Score Method for Lexical Weight.     [optional]\n"
            <<"                      Parameter is \"lexw\" or \"noisyor\".\n"
            <<"                      Default is \"lexw\".\n"
			<<"    -sortPhrase  :  Sort -tab and -tabinv                [optional]\n"
			<<"                      If we sorted extracted phrase table, but failed in next step, we can set 'no sort' to restart.\n"
			<<"                      Default is 1(true)\n"
            <<"[EXAMPLE]\n"
#ifdef WIN32
            <<"           NiuTrans.PhraseExtractor --SCORE -tab -tabinv -ls2d -ld2s -out\n" 
            <<"                                    [-cutoffInit] [-cutoffHiero] [-sort] [-temp]\n"
            <<"                                    [-printAlign] [-printFreq] [-method]\n"
#else
            <<"           NiuTrans.PhraseExtractor --SCORE -tab -tabinv -ls2d -ld2s -out\n"
            <<"                                    [-cutoffInit] [-cutoffHiero] [-temp]\n"
            <<"                                    [-printAlign] [-printFreq] [-method]\n"
#endif
            <<flush;
        return;
}

void GetParameter::getParamsOfScoreSyn()       // add in 2012-05-22, scoring for syntax-rule.
{
        cerr<<"####################################################################\n"
            <<"#   Scoring for Syntax-Rule (version 4.0)       --www.nlplab.com   #\n"
            <<"####################################################################\n"
            <<"[USAGE]\n"
            <<"            NiuTrans.PhraseExtractor    --SCORESYN       [OPTIONS]\n"
            <<"[OPTION]\n"
            <<"    -rule        :  Syntax  Rule.\n"
            <<"    -ls2d        :  Lex File s2d.\n"
            <<"    -ld2s        :  Lex File d2s.\n"
            <<"    -out         :  Output  File.\n"
#ifdef WIN32
            <<"    -sort        :  Cygwin Sort Dir.                     [optional]\n"
            <<"                      Default is \"c:\\cygwin\\bin\\sort.exe\".\n"
#endif
            <<"    -model       :  \"t2s\", \"s2t\", or \"t2t\"               [optional]\n"
            <<"                      Default is \"t2s\"\n"
            <<"    -temp        :  Sort Temp Dir.                       [optional]\n"
            <<"                      Default is system temp dir.\n"
            <<"    -cutoff      :  Cut Off Initial Phrase.              [optional]\n"
            <<"                      Default is 0.\n"
            <<"    -spormy      :  Speed or memory priority.            [optional]\n"
            <<"                      Parameter is \"memory\" or \"speed\".\n"
            <<"                      Default is \"memory\".\n"
            <<"    -lowerfreq   :  Below this value is considered       [optional]\n"
            <<"                    to be low frequency.\n"
            <<"                      Default is \"3\".\n"
            <<"[EXAMPLE]\n"
#ifdef WIN32
            <<"        NiuTrans.PhraseExtractor --SCORESYN -ls2d FILE -ld2s FILE\n"
            <<"                                            -rule FILE -out  FILE\n" 
            <<"                                            [-cutoff] [-sort] [-temp]\n"
#else
            <<"        NiuTrans.PhraseExtractor --SCORESYN -ls2d FILE -ld2s FILE\n"
            <<"                                            -rule FILE -out  FILE\n"
            <<"                                            [-cutoff] [-temp]\n"
#endif            
            <<flush;

        return;
}

void GetParameter::getParamsOfFilterWithNum()
{
        cerr<<"####################################################################\n"
            <<"#   Filter Rule with Number ( version 3.0 )    -- www.nlplab.com   #\n"
            <<"####################################################################\n"
            <<"[USAGE]\n"
            <<"            NiuTrans.PhraseExtractor    --FILTN    [OPTIONS]\n"
            <<"[OPTION]\n"
            <<"    -in            :  Input   File.\n"
            <<"    -out           :  Output  File.\n"
            <<"    -strict        :  Strict  Number.              [optional]\n"
            <<"                        Default is 30.\n"
            <<"    -tableFormat   :  Input Table Format.          [optional]\n"
            <<"                        Default is \"auto\".\n"
            <<"                        Parameters, \"phrase\", \"hierarchy\", \"syntax\" or \"SAMT\".\n"
            <<"[EXAMPLE]\n"
            <<"    >> Filter Phrase Table with Number\n"
            <<"           NiuTrans.PhraseExtractor --FILTN -in input-file -out output-file\n"
            <<"                                           [-strict 30]\n"
            <<"                                           [-tableFormat auto]\n"
            <<flush;
        return;

}

void GetParameter::getParamsOfFilterWithDev()
{
        cerr<<"####################################################################\n"
            <<"# Filter PhraseTable with Dev Set ( version 2.0 ) --www.nlplab.com #\n"
            <<"####################################################################\n"
            <<"[USAGE]\n"
            <<"            NiuTrans.PhraseExtractor    --FILPD   [OPTIONS]\n"
            <<"[OPTION]\n"
            <<"    -dev    :  Development Set.\n"
            <<"    -in     :  Input   File.\n"
            <<"    -out    :  Output  File.\n"
            <<"    -maxlen :  Strict for Max Phrase Length.      [optional]\n"
            <<"                 Defalut is 10.\n"
            <<"    -rnum   :  Ref Number in Dev Set.             [optional]\n"
            <<"                 Default is 4.\n"
            <<"[EXAMPLE]\n"
            <<"    >> Filter Phrase Table with Dev Set\n"
            <<"           NiuTrans.PhraseExtractor --FILPD -dev -in -out [-maxlen] [-rnum]\n\n"
            <<flush;
}

bool GetParameter::getParameters( int argc, string argv )
{
    if( argc == 2 )
    {
        if( argv == "--LEX" )
            getParamsOfGeneLexiWei();
        else if( argv == "--EXTP" )
            getParamsOfExtractPhrasePairs();
        else if( argv == "--EXTH" )
            getParamsOfExtractHieroRules();
        else if( argv == "--SCORE" )
            getParamsOfScore();
        else if( argv == "--SCORESYN" )          // add in 2012-05-22, scoring for syntax-rule.
            getParamsOfScoreSyn();
        else if( argv == "--FILTN" )
            getParamsOfFilterWithNum();
        else if( argv == "--FILPD" )
            getParamsOfFilterWithDev();
        return true;
    }
    return false;
}

bool GetParameter::allFunction( int argc )
{
    if( argc <= 1 )
    {
        cerr<<"####################################################################\n"
            <<"#   NiuTrans.PhraseExtractor (version 1.1.0)   -- www.nlplab.com   #\n"
            <<"####################################################################\n"
            <<"[USAGE]\n"
            <<"      NiuTrans.PhraseExtractor     <action>       [OPTIONS]\n"
            <<"[ACTION]\n"
            <<"      --LEX      :  Extract Lexical Table.\n"
            <<"      --EXTP     :  Extract Phrase Table.\n"
            <<"      --EXTH     :  Extract Hierarchy Phrase Table.\n"
            <<"      --SCORE    :  Scoring for (Hierarchy) Phrase Table.\n"
            <<"      --SCORESYN :  Scoring for Syntax Rule.\n"
            <<"      --FILTN    :  Filter Translation Table with Number Strict.\n"
            <<"      --FILPD    :  Filter Translation Table with Development Set.\n"
            <<"[TOOLS]\n"
            <<"    >> Get Options of Extract Lexical Table\n"
            <<"                 NiuTrans.PhraseExtractor     --LEX\n"
            <<"    >> Get Options of Extract Phrase Table\n"
            <<"                 NiuTrans.PhraseExtractor     --EXTP\n" 
            <<"    >> Get Options of Extract Hierarchy Phrase Table\n"
            <<"                 NiuTrans.PhraseExtractor     --EXTH\n" 
            <<"    >> Get Options of Scoring for (Hierarchy) Phrase Table\n"
            <<"                 NiuTrans.PhraseExtractor     --SCORE\n"
            <<"    >> Get Options of Scoring for Syntax Rule\n"
            <<"                 NiuTrans.PhraseExtractor     --SCORESYN\n"
            <<"    >> Get Options of Filter Translation Table with Number\n"
            <<"                 NiuTrans.PhraseExtractor     --FILTN\n"
            <<"    >> Get Options of Filter Translation Table with Development Set\n"
            <<"                 NiuTrans.PhraseExtractor     --FILPD\n\n"
            <<flush;
        return true;
    }
    return false;

}

bool GetParameter::allFunction( int argc, string argv )
{
    if( ( argc % 2 != 0 ) || ( argc % 2 == 0 ) && 
      ( ( argv != "--LEX"      ) && 
        ( argv != "--EXTP"     ) &&
        ( argv != "--EXTH"     ) &&
        ( argv != "--SCORE"    ) &&
        ( argv != "--FILTN"    ) &&
        ( argv != "--FILPD"    ) &&
        ( argv != "--SCORESYN" )    ) )      // add in 2012-05-22, scoring for syntax-rule.
    {
        cerr<<"####################################################################\n"
            <<"#   NiuTrans.PhraseExtractor ( version 2.0 )   -- www.nlplab.com   #\n"
            <<"####################################################################\n"
            <<"[USAGE]\n"
            <<"        NiuTrans.PhraseExtractor   <action>     [OPTIONS]\n"
            <<"[ACTION]\n"
            <<"    --LEX     :  Generate Lexical Weighting\n"
            <<"    --EXTP    :  Extract Phrase Pairs\n"
            <<"    --EXTH    :  Extract Hiero Rules\n"
            <<"    --SCORE   :  Score Phrase Translation Table\n"
            <<"    --FILTN   :  Filter Translation Table with Number Strict\n"
            <<"    --FILPD   :  Filter Phrase Translation Table with Dev Set\n"
            <<"[TOOLS]\n"
            <<"    >> Get Options of Generate Lexical Weighting\n"
            <<"                 NiuTrans.PhraseExtractor     --LEX\n"
            <<"    >> Get Options of Extract Phrase Pairs\n"
            <<"                 NiuTrans.PhraseExtractor     --EXTP\n" 
            <<"    >> Get Options of Extract Hiero Rules\n"
            <<"                 NiuTrans.PhraseExtractor     --EXTH\n" 
            <<"    >> Get Options of Scorer Function\n"
            <<"                 NiuTrans.PhraseExtractor     --SCORE\n"
            <<"    >> Get Options of Filter Translation Table with Number\n"
            <<"                 NiuTrans.PhraseExtractor     --FILTN\n"
            <<"    >> Get Options Filter Phrase Table with Dev Set\n"
            <<"                 NiuTrans.PhraseExtractor     --FILPD\n\n"
            <<flush;
        return true;
    }
    return false;
}
