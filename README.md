# NiuTrans.SMT: A Statistical Machine Translation System

* NiuTrans.SMT is an open-source statistical machine translation system developed by the joint team from the Natural Language Processing Lab. at Northeastern University and the YaTrans Co.,Ltd. The NiuTrans.SMT system is fully developed in C++ language. So it runs fast and uses less memory. Currently it supports phrase-based, hierarchical phrase-based and syntax-based (string-to-tree, tree-to-string and tree-to-tree) models for research-oriented studies.

## Features
1. Written in C++. So it runs fast.
2. Multi-thread supported
3. Easy-to-use APIs for feature engineering
4. Competitive performance for translation tasks
5. A compact but efficient n-gram language model is embedded. It does not need external support from other softwares (such as SRILM)
6. Supports multiple SMT models
	* Phrase-based model
	* Hierarchical phrase-based model
	* Syntax-based (string-to-tree, tree-to-string and tree-to-tree) models

## Requirements
* For Windows users, Visual Studio 2008, Cygwin, and perl (version 5.10.0 or higher) are required. It is suggested to install cygwin under path "C:\" by default. 

* For Linux users, gcc (version 4.1.2 or higher), g++ (version 4.1.2 or higher), GNU Make (version 3.81 or higher) and perl (version 5.8.8 or higher) are required.

NOTE: 2GB memory and 10GB disc space is a minimal requirement for running the system. Of course, more memory and disc space is helpful if the system is trained using large-scale corpus. To support large data/model (such as n-gram LM), 64bit OS is recommended. 

## Installation
### For Windows users   
	- open "NiuTrans.sln" in "NiuTrans\src\"
	- set configuration mode to "Release"
	- set platform mode to "Win32" (for 32bit OS) or "x64" (for 64bit OS)
	- build the whole solution
	 You will then find that all binaries are generated in "NiuTrans\bin\".
### For Linux users
    - cd NiuTrans/src/
    - chmod a+x install.sh 
    - ./install.sh -m32 (for 32bit OS) or ./install.sh (for 64bit OS)
    - source ~/.bashrc
     You will then find that all binaries are generated in "NiuTrans/bin/".

## Manual

The package also offers a manual to describe more details about the system, as well as various tricks to build better MT engines using NiuTrans. Click [here](https://developer.niutrans.com/niutrans.smt/niutrans-manual.pdf) to download the manual in pdf.

## NiuTrans Team
* [Jingbo Zhu](http://www.nlplab.com/members/zhujingbo.html)（Co-PI）
* [Tong Xiao](http://www.nlplab.com/members/xiaotong.html)（Co-PI）
* Yinqiao Li
* Quan Du
* Qiang Wang
* Yufan Jiang
* Ye Lin
* Yuhao Zhang

Acknowledgements: In the process of the implementation of this project, we get the support of previous graduates, they are Qiang Li (phrase extraction and many scripts), Hao Zhang (decoder, ME-reordering model), Rushan Chen (language model), Shujie Yao (data selection and data preprocessing), Ji Ma (language model and CWMT2013 baseline systems), Kunjie Sun (CWMT2013 Chinese-English baseline system) and Zhuo Liu (CWMT2013 English-Chinese baseline system).

## How To Cite NiuTrans

If you use NiuTrans.SMT in your research and would like to acknowledge this project, please cite the following paper

**Tong Xiao, Jingbo Zhu, Hao Zhang and Qiang Li. 2012. NiuTrans: An Open Source Toolkit for Phrase-based and Syntax-based Machine Translation. In Proc. of ACL, demonstration session.**

## Get Support
For any questions about NiuTrans, please e-mail to us (niutrans@mail.neu.edu.cn) directly.

## History
NiuTrans version 1.4.0 Beta - May 12, 2018 (bug fixes)  
NiuTrans version 1.3.1 Beta - December 1, 2014 (bug fixes for the t2s/t2t decoder and syntactic rule extraction module)  
NiuTrans version 1.3.0 Beta - July 17, 2013 (bug fixes, decoder updates, data preprocessing system updates, new scripts for CWMT2013)  
NiuTrans version 1.2.0 Beta - January 31, 2013 (bug fixes, decoder updates, add preprocessing system, word-alignment tool and recasing module)  
NiuTrans version 1.1.0 Beta - August 1, 2012 (bug fixes)  
NiuTrans version 1.0.0 Beta - July 7, 2012 (three syntax-based models are supported)  
NiuTrans version 0.3.0 - April 27, 2012 (hierarchical phrase-based model is supported)  
NiuTrans version 0.2.0 - October 29, 2011 (bug-fixing, 32bit OS supported)	 
NiuTrans version 0.1.0 - July 5, 2011 (first version)  

## Acknowledgements
This project is supported in part by the National Science Foundation of China, Specialized Research Fund for the Doctoral Program of Higher Education, and the Fundamental Research Funds for the Central Universities.



