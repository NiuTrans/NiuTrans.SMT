# NiuTrans统计机器翻译系统

* NiuTrans是一个开源的统计机器翻译系统，由中国的 [东北大学](http://www.neu.edu.cn/) |[自然语言处理实验室](http://www.nlplab.com/) 开发。NiuTrans系统全部由C++开发，运行速度快，占用内存少。目前该系统支持基于（层次）短语/基于句法的模型，可供研究之用。


## 系统特色
1. 由C++编写，运行速度快。
2. 支持多线程
3. 简单易用的API
4. 翻译任务中高质量的翻译效果
5. 内嵌小巧、高效的N-元语言模型，无需其它软件（如SRILM）的外部支持。
6. 支持多个统计机器翻译模型
	- 基于短语的模型
	* 基于层次短语的模型
	* 基于句法（树到串，串到树，树到树）的模型
  
## 下载
本系统为开放源码系统，依照GNU公用许可证[(GPL)](http://www.gnu.org/licenses/gpl-2.0.html)发布。
请点击[下载地址](http://183.129.153.70:5010/ntopen_server/niutrans-download-page.jsp)下载本系统的源代码和样本数据。

## 系统运行要求
* Windows用户：要求Visual Studio 2008，Cygwin，以及 perl（5.10.0 版本或更高）。建议默认在“C:\”路径下安装cygwin。

* Linux用户：要求gcc（4.1.2版本或更高），g++（4.1.2版本或更高），GNU Make(3.81版本或更高)以及perl（5.8.8 版本或更高）。

注意：运行本系统的最低要求为`2GB内存、10GB硬盘空间`。 如果使用大规模的语料训练本系统，则需要更多的内存和硬盘空间。建议使用64位操作系统以支持大规模语料/模型（如N-元语言模型） 

## 安装
请解压下载的软件包（假设安装目录是“NiuTrans”），按照如下说明安装本系统。

### 对于Windows用户   
	- 在 NiuTrans\src\下打开NiuTrans.sln
	- 设置编译模式为Release
	- 设置平台模式为Win32（32位OS）或x64（64位OS）
	- 构建整体解决方案
	- 在NiuTrans\bin\目录下生成了所有的二进制文件。
### 对于Linux用户
    - $ cd NiuTrans/src/
    - $ chmod a+x install.sh 
    - $ ./install.sh -m32 (32位OS) 或 ./install.sh (64位OS)
    - $ source ~/.bashrc
    - 在“NiuTrans/bin/”目录下生成了所有的二进制文件。

## 手册
在NiuTrans中提供描述更多关于NiuTrans细节的手册，同时介绍如何使用NiuTrans更好搭建统计机器翻译系统。点击这里下载[ PDF版手册 ](http://www.niutrans.com/niutrans/man/niutrans-manual.pdf)。

## NiuTrans团队
* [朱靖波](http://www.nlplab.com/members/zhujingbo.html)（项目负责人）
* [肖桐](http://www.nlplab.com/members/xiaotong.html)（技术负责人）
* [李强](http://www.nlplab.com/members/liqiang.html)
* 杜权
* 王强

致谢：在本项目实施的过程中，获得了往届毕业生的支持，他们是：张浩（解码器，调序模型），陈如山（语言模型），姚树杰（数据选取和数据预处理），马骥（语言模型，CWMT2013基线系统），孙坤杰（CWMT2013汉英基线系统），刘卓（CWMT2013英汉基线系统）。

## 如何引用NiuTrans
如果您在研究中使用了NiuTrans，请在参考文献中注明引用如下论文

**Tong Xiao, Jingbo Zhu, Hao Zhang and Qiang Li. 2012. NiuTrans: An Open Source Toolkit for Phrase-based and Syntax-based Machine Translation. In Proc. of ACL, demonstration session.**

## 获得帮助
有关NiuTrans, 请发送e-mail至niutrans@mail.neu.edu.cn。我们将为您详细解答。

## NiuTrans历史
* NiuTrans 1.3.1 Beta版 - 2014年12月1日（解码器及句法规则抽取bug修复）
* NiuTrans 1.3.0 Beta版 - 2013年7月17日（bug修复，解码器升级，数据预处理系统升级，增加CWMT2013评测专用脚本
* NiuTrans 1.2.0 Beta版 - 2013年1月31日（bug修复，解码器升级，增加数据预处理系统，增加词对齐工具，增加recasing模块）
* NiuTrans 1.1.0 Beta版 - 2012年8月1日（bug修复）
* NiuTrans 1.0.0 Beta版 - 2012年7月7日（支持三套基于句法的模型）
* NiuTrans 0.3.0版 - 2012年4月27日（支持基于层次短语的模型）
* NiuTrans 0.2.0版 - 2011年10月29日（bug修复，支持32位操作系统）
* NiuTrans 0.1.0版 - 2011年7月5日（第一个版本）

## 致谢
本项目的部分工作获得了如下支持：国家自然科学基金(60873091; 61073140)， 高等院校博士学科点专项科研基金(20100042110031), 以及中央高校基本科研基金。


截止2018年5月1日， NiuTrans累计下载`3000余次`。

---
# 使用说明

## 使用自己的数据-Linux(目前仅支持中英/英中翻译任务)

概述：双语训练数据/开发集/测试集/语言模型训练数据处理步骤

这里首先介绍一下数据预处理的基本内容。每个步骤的具体操作细节在后面会有详细描述。

* 双语训练数据
```
	步骤1：使用 NiuTrans-clear.illegal.char.pl 脚本进行双语乱码过滤，具体指令见“2. 双语训练数据乱码过滤”
	
	步骤2：对过滤的双语文本分别进行数据预处理操作，具体指令见“4/5. 中文/英文数据预处理”
	
	注 意：进行数据预处理时，开启泛化开关，泛化数字($number)、时间($time)、日期($date)，关闭翻译开关。
           即 NiuTrans-running-segmenter.pl 中 -method 参数值为 01
	
	步骤3: 利用自动词对齐工具得到双语句子的词到词次对应关系（具体指令见步骤“6. 开发集格式处理”）
```

* 开发集
```
	步骤1：对源语言文本进行数据预处理。源语言文本进行数据预处理时，同时开启泛化开关与翻译开关，
		   即 NiuTrans-running-segmenter.pl 中 -method 参数值为 11，泛化并翻译数字、时间、日期
	
	步骤2：对多个参考翻译结果文本进行数据预处理。参考翻译结果进行数据预处理时，关闭泛化开关与翻译开关。
	       即 NiuTrans-running-segmenter.pl 中 -method 参数值为 00
	
	步骤3：进行开发集格式处理，处理为NiuTrans使用的开发集数据格式，具体见步骤“7. 词对齐”。
```

* 测试集
```
	步骤1：对源语言文本进行数据预处理。源语言文本进行数据预处理时，同时开启泛化开关与翻译开关，
	       即 NiuTrans-running-segmenter.pl 中 -method 参数值为 11，泛化并翻译数字、时间、日期
```

* 语言模型训练数据
```
	步骤1：将双语训练数据的目标语言端文本与准备的大规模目标语言端单语语料进行合并
	
	步骤2：对语言模型训练数据进行数据预处理。进行数据预处理时，开启泛化开关，关闭翻译开关。
	       即 NiuTrans-running-segmenter.pl 中 -method 参数值为 01
```

## 1. 示例数据

* NiuTrans系统支持用户使用自己的双语数据搭建机器翻译系统，这里提供了1000句的示例数据：

  NiuTrans软件包中提供了一些示例数据，供连通预处理系统与词对齐系统之用，位于"NiuTrans/sample-data/sample-submission-version"文件夹中。

    sample-submission-version/
      -- Raw-data/                          # 原始双语句对集（共有1,000句对）
           -- chinese.txt                   # 源语句子集
           -- english.txt                   # 目标语句子集

* 格式：请解压“NiuTrans/sample-data/sample.tar.gz”，有关数据格式的更多信息参见“description-of-the-sample-data”。

## 2.双语训练数据乱码过滤

* 操作说明（要求安装perl。此外， Windows用户要求安装Cygwin）

```
$> cd NiuTrans/sample-data/
$> tar xzvf sample.tar.gz                      # 如果“sample.tar.gz”已经解压缩，则忽略此步
$> cd NiuTrans/scripts/
$> mkdir ../work/preprocessing -p
$> perl NiuTrans-clear.illegal.char.pl \
        -src    ../sample-data/sample-submission-version/Raw-data/chinese.raw.txt \
        -tgt    ../sample-data/sample-submission-version/Raw-data/english.raw.txt \
        -outSrc ../work/preprocessing/chinese.clean.txt \
        -outTgt ../work/preprocessing/english.clean.txt
```

  **在“NiuTrans-clear.illegal.char.pl”脚本中：** 

  “-src”表示中文文本（无论训练汉英/英汉翻译系统，该参数都指定为中文）。 

  “-tgt”表示英文文本（无论训练汉英/英汉翻译系统，该参数都指定为英文）。

  “-outSrc”表示输出过滤乱码后中文文本。

  “-outTgt”表示输出过滤乱码后英文文本。 

* 输出：在“/NiuTrans/work/preprocessing”目录下生成两个文件 

```
- chinese.clean.txt                            # 中文预处理文本
- english.clean.txt                            # 英文预处理文本
```

## 3.单语数据乱码过滤

* 操作说明（要求安装perl。此外， Windows用户要求安装Cygwin） 

```
$> cd NiuTrans/sample-data/
$> tar xzvf sample.tar.gz                      # 如果“sample.tar.gz”已经解压缩，则忽略此步
$> cd NiuTrans/scripts/
$> mkdir ../work/preprocessing -p
$> perl NiuTrans-monolingual.clear.illegal.char.pl \
		-tgt    ../sample-data/sample-submission-version/Raw-data/chinese.raw.txt \
		-outTgt ../work/preprocessing/chinese.mono.clean.txt \
		-lang zh
```

**在“NiuTrans-monolingual.clear.illegal.char.pl”脚本中：**

“-tgt”表示输入待处理文本。

“-outTgt”表示输出过滤乱码后文本。

“-lang”表示输出文本语言，zh为中文，en为英文。

* 输出：在“/NiuTrans/work/preprocessing”目录下生成一个文件

```
- chinese.mono.clean.txt                            # 中文预处理文本
```

## 4.中文数据预处理

* 操作说明（要求安装perl。此外， Windows用户要求安装Cygwin）

```
	$> perl NiuTrans-running-segmenter.pl \        # 中文预处理
		-lang   ch \
		-input  ../work/preprocessing/chinese.clean.txt \
		-output ../work/preprocessing/chinese.clean.txt.prepro \
		-method 01
```

**在“NiuTrans-running-segmenter.pl”脚本中：**

“-lang”表示数据预处理文件类型，“-lang”参数值为“ch”。

“-input”为输入待处理文本。

“-output”为输出预处理后文本。

“-method”表示数据预处理中泛化和翻译的开关。参数值为“00”时，不泛化也不翻译；参数值为“01”时，只泛化不翻译；参数值为“11”时，泛化并翻译。这里的泛化指的是训练数据中数字（$number）、时间（$time）、日期（$date）的泛化。 

* 输出：在“/NiuTrans/work/preprocessing”目录下生成一个文件

```
- chinese.clean.txt.prepro                     # 中文预处理文本
```

## 5.英文数据预处理

* 操作说明（要求安装perl。此外， Windows用户要求安装Cygwin）

```
	$> perl NiuTrans-running-segmenter.pl \        # 英文预处理
		-lang   en \
		-input  ../work/preprocessing/english.clean.txt \
		-output ../work/preprocessing/english.clean.txt.prepro \
		-method 01
```

**在“NiuTrans-running-segmenter.pl”脚本中：**

“-lang”为预处理文件类型，“-lang”参数值为“en”。

“-input”为输入待处理文本。

“-output”为输出预处理后文本。

“-method”表示数据预处理中泛化和翻译的开关。参数值为“00”时，不泛化也不翻译；参数值为“01”时，只泛化不翻译；参数值为“11”时，泛化并翻译。这里的泛化指的是训练数据中数字（$number）、时间（$time）、日期（$date）的泛化。 

* 输出：在“/NiuTrans/work/preprocessing”目录下生成一个文件

```
- english.clean.txt.prepro                     # 英文预处理文本
```

## 6.开发集格式处理

* 操作说明（要求安装perl。此外， Windows用户要求安装Cygwin）

```
$> cd NiuTrans/scripts/
$> perl NiuTrans-dev-merge.pl \                   # 生成开发集文本
		原文  \
		译文1 \
		译文2 \
		译文3 \
		译文4 \
		...   \
		> ../work/preprocessing/dev.txt
```

* NiuTrans系统进行权重调优时，使用的开发集格式为：1行源语，1行空行（句法树），目标语参考翻译结果（多个多行）。双语平行句对经过数据预处理后，需要使用上述指令进行格式处理。若用户有多个译文，可直接写入命令行中。

* 输出：在“/NiuTrans/work/preprocessing”目录下生成一个开发集文本

```
	- dev.txt                                          # 开发集文本
```

## 7.词对齐

* 下载链接：[http://code.google.com/p/giza-pp/downloads/detail?name=giza-pp-v1.0.7.tar.gz](http://code.google.com/p/giza-pp/downloads/detail?name=giza-pp-v1.0.7.tar.gz)，将下载的“giza-pp-v1.0.7.tar.gz”放入“/NiuTrans/tools”目录下

* 使用说明

```
$> cd NiuTrans/tools/
$> tar xzvf giza-pp-v1.0.7.tar.gz
$> cd giza-pp
$> make
$> cp GIZA++-v2/GIZA++ \                         # 拷贝“GIZA++”,“snt2cooc.out”,“plain2snt.out”,
		GIZA++-v2/snt2cooc.out \                   # “mkcls”至“/NiuTrans/bin”目录下
		GIZA++-v2/plain2snt.out \
		mkcls-v2/mkcls \
		../../bin                                      
$> cd ../../scripts
$> mkdir ../work/wordalignment/ -p
$> nohup nice perl NiuTrans-running-GIZA++.pl \  # 此步比较耗时，采用后台方式运行，查看运行情况“tail -f nohup.out”
		-src    ../work/preprocessing/chinese.clean.txt.prepro \
		-tgt    ../work/preprocessing/english.clean.txt.prepro \
		-out    ../work/wordalignment/alignment.txt \
		-tmpdir ../work/wordalignment/ &
```

“-src”表示预处理后的中文文本。

“-tgt”表示预处理后的英文文本。

“-out”表示经过对称化处理的词对齐文件。

* 输出：在“/NiuTrans/work/wordalignment”目录下生成词对齐结果文件

```
- alignment.txt                                # 对称化处理的词对齐文本
```

## 短语系统使用说明

## 1.准备数据

* NiuTrans翻译系统是“数据驱动”的MT系统，要求使用数据对系统进行训练和/或调参。在运行该系统前要求用户准备好以下数据：

	a).训练数据：双语句对以及对应的词对齐结果
	
	b).开发数据：带有至少一个参考译文的源语句子集
	
	c).测试数据：新句集
	
	d).评价数据：测试数据集的参考答案
	
NiuTrans软件包中提供了一些样本文件，供系统实验以及研究格式要求之用，位于"NiuTrans/sample-data/sample-submission-version"文件夹中。
	
```
sample-submission-version/
  -- TM-training-set/                   # 带有词对齐结果的双语句对集（共有100,000句对）
       -- chinese.txt                   # 源语句子集
       -- english.txt                   # 目标语句子集（英文全部转换为小写字符）
       -- Alignment.txt                 # 句对的词对齐结果
  -- LM-training-set/
       -- e.lm.txt                      # 单语语料，用于训练语言模型（100K目标语句子）
  -- Dev-set/
       -- Niu.dev.txt                   # 开发数据集，用于权重调优（400句子集）
  -- Test-set/
       -- Niu.test.txt                  # 测试数据集（1K句子集）
  -- Reference-for-evaluation/
       -- Niu.test.reference            # 测试数据集的参考译文（1K句子集）
  -- Recaser-training-set/
       -- english.keepcase.txt          # 单语语料，用于训练Recasing模型（10K句子集）
  -- description-of-the-sample-data     # 样本数据的描述
```

* 格式：请解压“NiuTrans/sample-data/sample.tar.gz”，有关数据格式的更多信息参见“description-of-the-sample-data”。

* 以下各步骤中使用上述数据集来讲解如何运行NiuTrans系统（如何训练MT模型，调整特征权重，以及对测试句进行解码等）。

## 2.训练翻译模型

* 操作说明（要求安装perl。此外， Windows用户要求安装Cygwin）

```
$> cd NiuTrans/sample-data/
$> tar xzf sample.tar.gz              # 如果“sample.tar.gz”已经解压缩，则忽略此步
$> cd ../
$> mkdir work/model.phrase/ -p
$> cd scripts/
$> perl NiuTrans-phrase-train-model.pl \
        -tmdir ../work/model.phrase/ \
        -s ../sample-data/sample-submission-version/TM-training-set/chinese.txt \
        -t ../sample-data/sample-submission-version/TM-training-set/english.txt \
        -a ../sample-data/sample-submission-version/TM-training-set/Alignment.txt
```

“-tmdir”为生成各种表和模型文件的目标目录

“-s”，“-t”和“-a”分别指源语句子、目标语句子和二者的对齐结果（每行一句）

* 输出：在“NiuTrans/work/model.phrase/”目录中生成3个文件

```
- me.reordering.table                 # ME调序模型
- msd.reordering.table                # MSD调序模型
- phrase.translation.table            # 短语翻译模型
```

* 注意：在运行“NiuTrans-phrase-train-model.pl”前请进入“NiuTrans/scripts/”目录。

## 3.训练N元语言模型

* 操作说明

```
$> cd ../
$> mkdir work/lm/
$> cd scripts/
$> perl NiuTrans-training-ngram-LM.pl \
        -corpus ../sample-data/sample-submission-version/LM-training-set/e.lm.txt \
        -ngram  3 \
        -vocab  ../work/lm/lm.vocab \
        -lmbin  ../work/lm/lm.trie.data
```

“-ngram”为N元语言模型中的元数。如，“-ngram 3”指3元语言模型。

“-vocab”为所生成的目标语端语言模型词汇表。

“-lmbin”为所生成的目标语端语言模型的二进制文件。

* 输出： 生成两个文件，放在目录“NiuTrans/work/lm/”中

```
- lm.vocab                            # 目标语端词汇表
- lm.trie.data                        # 二进制语言模型
```

## 4.生成配置文件

* 操作说明

```
$> cd scripts/
$> perl NiuTrans-phrase-generate-mert-config.pl \ 
        -tmdir  ../work/model.phrase/ \
        -lmdir  ../work/lm/ \
        -ngram  3 \
        -o      ../work/NiuTrans.phrase.user.config
```

“-tmdir”为包含短语翻译表和调序模型文件的目录。

“-lmdir”为包含N元语言模型和目标语端词汇表的目录。

“-ngram”为N元语言模型中的元数。

“-o”为输出结果，即配置文件。

* 输出：生成配置文件并放在目录“NiuTrans/work/”下：

```
- NiuTrans.phrase.user.config           # 用于解码的配置文件
```

## 5.规则过滤

* 使用该步骤，可以极大提高您使用NiuTrans进行研究的效率；如果您对此步不感兴趣，可直接进行“6. 权重调优”。

* 操作说明（要求使用perl）

```
$> cd ..
$> cat sample-data/sample-submission-version/Dev-set/Niu.dev.txt \
       sample-data/sample-submission-version/Reference-for-evaluation/Niu.test.reference \
       > sample-data/sample-submission-version/Dev-set/Niu.dev.and.test.txt
$> bin/NiuTrans.PhraseExtractor --FILPD \
        -dev     sample-data/sample-submission-version/Dev-set/Niu.dev.and.test.txt \
        -in      work/model.phrase/phrase.translation.table \
        -out     work/model.phrase/phrase.translation.table.filterDevAndTest \
        -maxlen  10 \
        -rnum    1
$> perl scripts/filter.msd.model.pl \
        work/model.phrase/phrase.translation.table.filterDevAndTest \  
        work/model.phrase/msd.reordering.table \
        > work/model.phrase/msd.reordering.table.filterDevAndTest
```

“-dev”为权重调优的开发集，在这里使用的文件将开发集和测试集进行了合并。

“-in”为待过滤的短语翻译表。

“-out”为过滤后的短语翻译表。

“-maxlen”为短语翻译表中短语的最大长度。

“-rnum”为开发集中参考答案的个数，sample data中参考答案个数为1。 

* 输出：配置文件“NiuTrans/work/NiuTrans.phrase.user.config”中使用的短语翻译表及MSD调序模型重新配置。

```
param="MSD-Reordering-Model"   value="../work/model.phrase/msd.reordering.table"
修改为
param="MSD-Reordering-Model"   value="../work/model.phrase/msd.reordering.table.filterDevAndTest"
   
param="Phrase-Table"           value="../work/model.phrase/phrase.translation.table"
修改为
param="Phrase-Table"           value="../work/model.phrase/phrase.translation.table.filterDevAndTest"
```

## 6.权重调优

* 操作说明（要求使用perl）

```
$> perl NiuTrans-phrase-mert-model.pl \
        -dev  ../sample-data/sample-submission-version/Dev-set/Niu.dev.txt \
        -c    ../work/NiuTrans.phrase.user.config \
        -nref 1 \
        -r    2 \
        -l    ../work/mert-model.log
```

“-dev”为权重调优的开发集。

“-c”为以前的步骤中生成的配置文件。

“-nref”为每个源语句子提供的参考译文的数量。

“-r”为执行MERT程序的轮数 (初始设置为1轮 = 15 MERT 循环)。

“-l”为MERT生成的log文件。

* 输出：最优的特征权重被记录在配置文件“NiuTrans/work/NiuTrans.phrase.user.config”中。这些特征权重将被用于对测试句的解码过程中。

## 7.解码

* 操作说明（要求使用perl）

```
$> perl NiuTrans-phrase-decoder-model.pl \
        -test   ../sample-data/sample-submission-version/Test-set/Niu.test.txt \
        -c      ../work/NiuTrans.phrase.user.config \
        -output 1best.out
```

“-test”为测试数据集（每行一句）。

“-c”为配置文件。

“-output”为翻译结果文件（如果未规定此选项，则翻译结果将被放入“stdout”中）。

* 输出：在“NiuTrans/scripts/”目录下生成翻译结果文件

```
- 1best.out                         # 测试句子的1-best翻译结果
```

## 8.恢复大写信息(recasing)

* 操作说明（要求使用perl）

```
$> mkdir ../work/model.recasing -p
$> perl NiuTrans-training-recase-model.pl \
        -corpus   ../sample-data/sample-submission-version/Recaser-training-set/english.keepcase.txt \
        -modelDir ../work/model.recasing
$> perl NiuTrans-recaser.pl \
        -config ../work/model.recasing/recased.config.file \
        -test   1best.out \
        -output 1best.out.recased
```

“-corpus”为训练数据集（每行一句）。

“-modelDir”为模型文件及配置文件的生成目录。

* 输出：在“NiuTrans/work/model/recasing”目录下生成模型及配置文件

```
- recased.config.file               # Recasing模型配置文件
- recased.lm.trie.data              
- recased.lm.vocab
- recased.null
- recased.phrase.translation.table
```

输出：在“NiuTrans/scripts”目录下生成翻译结果文件

```
- 1best.out.recased   
```

* 注意：这个步骤仅对目标语为英文的翻译任务有效。

## 9. Detokenizer

* 操作说明（要求使用perl）

```
$> perl NiuTrans-detokenizer.pl \
        -in  1best.out.recased \
        -out 1best.out.recased.detoken
```

“-in”为输入文件。

“-output”为输出文件。

* 输出：在“NiuTrans/scripts/”目录下生成最终文件

```
- 1best.out.recased.detoken   
```

## 10.评价

* 操作说明（要求使用perl）

```
$> perl NiuTrans-generate-xml-for-mteval.pl \
        -1f   1best.out \
        -tf   ../sample-data/sample-submission-version/Reference-for-evaluation/Niu.test.reference \
        -rnum 1
$> perl mteval-v13a.pl \
        -r ref.xml \
        -s src.xml \
        -t tst.xml
```

“-1f”为测试句子集的1-best翻译结果文件。

“-tf”为测试句子集的源语言句子及参考翻译结果文件。

“-r”为参考翻译文件。

“-s”为源语句的文件。 

“-t”为MT系统生成的（1-best）翻译结果文件。

* 输出：显示IBM版的BLEU值。如果以上各个步骤正确，您将得到测试样本数据的BLEU值约为0.2412。

* 注意：脚本mteval-v13a.pl依赖XML::Parser包。如果您的系统中没有安装XML::Parser，请按照以下命令行进行安装。

```
$> su root
$> tar xzf XML-Parser-2.41.tar.gz
$> cd XML-Parser-2.41/
$> perl Makefile.PL
$> make install
```

## 层次短语系统使用说明

## 1.准备数据

* NiuTrans翻译系统是“数据驱动”的MT系统，要求使用数据对系统进行训练和/或调参。在运行该系统前要求用户准备好以下数据：

	a).训练数据：双语句对以及对应的词对齐结果
	
	b).开发数据：带有至少一个参考译文的源语句子集
	
	c).测试数据：新句集
	
	d).评价数据：测试数据集的参考答案

NiuTrans软件包中提供了一些样本文件，供系统实验以及研究格式要求之用，位于"NiuTrans/sample-data/sample-submission-version"文件夹中。

```
sample-submission-version/
  -- TM-training-set/                   # 带有词对齐结果的双语句对集（共有100,000句对）
       -- chinese.txt                   # 源语句子集
       -- english.txt                   # 目标语句子集（英文全部转换为小写字符）
       -- Alignment.txt                 # 句对的词对齐结果
  -- LM-training-set/
       -- e.lm.txt                      # 单语语料，用于训练语言模型（100K目标语句子）
  -- Dev-set/
       -- Niu.dev.txt                   # 开发数据集，用于权重调优（400句子集）
  -- Test-set/
       -- Niu.test.txt                  # 测试数据集（1K句子集）
  -- Reference-for-evaluation/
       -- Niu.test.reference            # 测试数据集的参考译文（1K句子集）
  -- description-of-the-sample-data     # 样本数据的描述
```

* 格式：请解压“NiuTrans/sample-data/sample.tar.gz”，有关数据格式的更多信息参见“description-of-the-sample-data”。

* 以下各步骤中使用上述数据集来讲解如何运行NiuTrans系统（如何训练MT模型，调整特征权重，以及对测试句进行解码等）。

## 2. 生成层次短语规则

* 操作说明（要求安装perl。此外， Windows用户要求安装Cygwin）

```
$> cd NiuTrans/sample-data/
$> tar xzf sample.tar.gz
$> cd ../
$> mkdir work/model.hierarchy/ -p
$> cd scripts/
$> perl NiuTrans-hierarchy-train-model.pl \
        -src ../sample-data/sample-submission-version/TM-training-set/chinese.txt \
        -tgt ../sample-data/sample-submission-version/TM-training-set/english.txt \
        -aln ../sample-data/sample-submission-version/TM-training-set/Alignment.txt \
        -out ../work/model.hierarchy/hierarchy.rule.table
```

“-out”为生成的层次短语规则表。

“-src”，“-tgt”和“-aln”分别为源语言，目标语言以及二者之间的词对齐文件（每行一句）。

* 输出：生成一个文件，放在目录“NiuTrans/work/model.hierarchy/”下

```
- hierarchy.rule.table                    # 层次短语规则表
```

* 注意：在运行“NiuTrans-hierarchy-train-model.pl”前请进入“NiuTrans/scripts/”目录。

## 3.训练N元语言模型

* 操作说明

```
$> cd ../
$> mkdir work/lm/
$> cd scripts/
$> perl NiuTrans-training-ngram-LM.pl \
        -corpus ../sample-data/sample-submission-version/LM-training-set/e.lm.txt \
        -ngram  3 \
        -vocab  ../work/lm/lm.vocab \
        -lmbin  ../work/lm/lm.trie.data
```

“-ngram”为N元语言模型中的元数。如，“-ngram 3”指3元语言模型。

“-vocab”为所生成的目标语端语言模型词汇表。

“-lmbin”为所生成的目标语端语言模型的二进制文件。

* 输出： 生成两个文件，放在目录“NiuTrans/work/lm/”中

```
- lm.vocab                            # 目标语端词汇表
- lm.trie.data                        # 二进制语言模型
```

## 4.生成配置文件

* 操作说明

```
$> cd scripts/
$> perl NiuTrans-hierarchy-generate-mert-config.pl \
        -rule  ../work/model.hierarchy/hierarchy.rule.table \
        -lmdir ../work/lm/ \
        -nref  1 \
        -ngram 3 \
        -out   ../work/NiuTrans.hierarchy.user.config
```

“-rule”为层次短语规则表。

“-lmdir”为包含N元语言模型和目标语端词汇表的目录。

“-ngram”为N元语言模型中的元数。

“-out”为输出结果，即配置文件。

* 输出：生成配置文件并放在目录“NiuTrans/work/”下：

```
- NiuTrans.hierarchy.user.config           # 用于MERT和解码的配置文件
```

## 5. 规则过滤

* 使用开发集和测试集对规则表进行过滤；如果您对此步不感兴趣，可直接进行“6. 权重调优”。

* 操作说明（要求使用perl）

```
$> cd ..
$> cat sample-data/sample-submission-version/Dev-set/Niu.dev.txt \
       sample-data/sample-submission-version/Reference-for-evaluation/Niu.test.reference \
       > sample-data/sample-submission-version/Dev-set/Niu.dev.and.test.txt
$> bin/NiuTrans.PhraseExtractor --FILPD \
        -dev     sample-data/sample-submission-version/Dev-set/Niu.dev.and.test.txt \
        -in      work/model.hierarchy/hierarchy.rule.table \
        -out     work/model.hierarchy/hierarchy.rule.table.filterDevAndTest \
        -maxlen  10 \
        -rnum    1
$> vim work/NiuTrans.hierarchy.user.config
   param="SCFG-Rule-Set"          value="../work/model.hierarchy/hierarchy.rule.table"
   修改为
   param="SCFG-Rule-Set"          value="../work/model.hierarchy/hierarchy.rule.table.filterDevAndTest"   
```

“-dev”为权重调优的开发集，在这里使用的文件将开发集和测试集进行了合并。

“-in”为待过滤的层次短语规则。

“-out”为过滤后的层次短语规则。

“-maxlen”为层次短语规则中规则最大长度。

“-rnum”为开发集中参考答案的个数，sample data中参考答案个数为1。 

* 输出：配置文件“NiuTrans/work/NiuTrans.hierarchy.user.config”中使用的层次短语规则表重新配置。

## 6.权重调优

* 操作说明（要求使用perl）

```
$> perl NiuTrans-hierarchy-mert-model.pl \
        -config ../work/NiuTrans.hierarchy.user.config \
        -dev    ../sample-data/sample-submission-version/Dev-set/Niu.dev.txt \
        -nref   1 \
        -round  2 \
        -log    ../work/mert-model.log
```

“-config”为以前的步骤中生成的配置文件。

“-dev”为权重调优的开发集。

“-nref”为每个源语句子提供的参考译文的数量。

“-round”为执行MERT程序的轮数 (初始设置为1轮 = 15 MERT 循环)。

“-log”为MERT生成的log文件。

* 输出：最优的特征权重被记录在配置文件“NiuTrans/work/NiuTrans.hierarchy.user.config”中。这些特征权重将被用于对测试句的解码过程中。

## 7.解码

* 操作说明（要求使用perl）

```
$> perl NiuTrans-hierarchy-decoder-model.pl \
        -config ../work/NiuTrans.hierarchy.user.config \
        -test   ../sample-data/sample-submission-version/Test-set/Niu.test.txt \
        -output 1best.out
```

“-config”为配置文件。

“-test”为测试数据集（每行一句）。

“-output”为翻译结果文件（如果未规定此选项，则翻译结果将被放入“stdout”中）。

* 输出：在“NiuTrans/scripts/”目录下生成翻译结果文件

```
- 1best.out                         # 测试句子的1-best翻译结果
```

## 8. 评价

* 操作说明（要求使用perl）

```
$> perl NiuTrans-generate-xml-for-mteval.pl \
        -1f   1best.out \
        -tf   ../sample-data/sample-submission-version/Reference-for-evaluation/Niu.test.reference \
        -rnum 1
$> perl mteval-v13a.pl \
        -r ref.xml \
        -s src.xml \
        -t tst.xml
```

“-1f”为测试句子集的1-best翻译结果文件。

“-tf”为测试句子集的源语言句子及参考翻译结果文件。

“-r”为参考翻译文件。

“-s”为源语句的文件。 

“-t”为MT系统生成的（1-best）翻译结果文件。

* 输出：显示IBM版的BLEU值。如果以上各个步骤正确，您将得到测试样本数据的BLEU值约为0.2417 （0.2386）。

* 注意：脚本mteval-v13a.pl依赖XML::Parser包。如果您的系统中没有安装XML::Parser，请按照以下命令行进行安装。

```
$> su root
$> tar xzf XML-Parser-2.41.tar.gz
$> cd XML-Parser-2.41/
$> perl Makefile.PL
$> make install
```

## 句法系统使用说明

## 1. 准备数据

* NiuTrans翻译系统是“数据驱动”的MT系统，要求使用数据对系统进行训练和/或调参。在运行该系统前要求用户准备好以下数据：

	a). 训练数据：双语句对以及对应的词对齐结果 
	
	b). 开发数据：带有至少一个参考译文的源语句子集
	
	c). 测试数据：新句集
	
	d). 评价数据：测试数据集的参考答案

NiuTrans软件包中提供了一些样本文件，供系统实验以及研究格式要求之用，位于"NiuTrans/sample-data/sample-submission-version"文件夹中。

```
sample-submission-version/
  -- TM-training-set/                   # 带有词对齐结果的双语句对集（共有100,000句对）
       -- chinese.txt                   # 源语句子集
       -- english.txt                   # 目标语句子集（英文全部转换为小写字符）
       -- Alignment.txt                 # 句对的词对齐结果
       -- chinese.tree.txt              # 源语言句法树
       -- english.tree.txt              # 目标语言句法树
  -- LM-training-set/
       -- e.lm.txt                      # 单语语料，用于训练语言模型（100K目标语句子）
  -- Dev-set/
       -- Niu.dev.txt                   # 开发数据集，用于权重调优（400句子集）
       -- Niu.dev.tree.txt              # 包含源语言端句法树信息的开发集
  -- Test-set/
       -- Niu.test.txt                  # 测试数据集（1K句子集）
       -- Niu.test.tree.txt             # 包含句法树信息的测试数据集
  -- Reference-for-evaluation/
       -- Niu.test.reference            # 测试数据集的参考译文（1K句子集）
  -- description-of-the-sample-data     # 样本数据的描述
```

* 格式：请解压“NiuTrans/sample-data/sample.tar.gz”，有关数据格式的更多信息参见“description-of-the-sample-data”。

* 以下各步骤中使用上述数据集来讲解如何运行NiuTrans系统（如何训练MT模型，调整特征权重，以及对测试句进行解码等）。

## 2. 生成句法规则

* 操作说明（要求安装perl。此外， Windows用户要求安装Cygwin）

串到句法树

```
$> cd NiuTrans/sample-data/
$> tar xzf sample.tar.gz
$> cd ../
$> mkdir work/model.syntax.s2t/ -p
$> cd scripts/
$> perl NiuTrans-syntax-train-model.pl \
        -model s2t \
        -src   ../sample-data/sample-submission-version/TM-training-set/chinese.txt \
        -tgt   ../sample-data/sample-submission-version/TM-training-set/english.txt \
        -aln   ../sample-data/sample-submission-version/TM-training-set/Alignment.txt \
        -ttree ../sample-data/sample-submission-version/TM-training-set/english.tree.txt \
        -out   ../work/model.syntax.s2t/syntax.string2tree.rule
```

句法树到串

```
$> cd NiuTrans/sample-data/
$> tar xzf sample.tar.gz
$> cd ../
$> mkdir work/model.syntax.t2s/ -p
$> cd scripts/
$> perl NiuTrans-syntax-train-model.pl \
        -model t2s \
        -src   ../sample-data/sample-submission-version/TM-training-set/chinese.txt \
        -stree ../sample-data/sample-submission-version/TM-training-set/chinese.tree.txt \
        -tgt   ../sample-data/sample-submission-version/TM-training-set/english.txt \
        -aln   ../sample-data/sample-submission-version/TM-training-set/Alignment.txt \
        -out   ../work/model.syntax.t2s/syntax.tree2string.rule
```

句法树到句法树

```
$> cd NiuTrans/sample-data/
$> tar xzf sample.tar.gz
$> cd ../
$> mkdir work/model.syntax.t2t/ -p
$> cd scripts/
$> perl NiuTrans-syntax-train-model.pl \
        -model t2t \
        -src   ../sample-data/sample-submission-version/TM-training-set/chinese.txt \
        -stree ../sample-data/sample-submission-version/TM-training-set/chinese.tree.txt \
        -tgt   ../sample-data/sample-submission-version/TM-training-set/english.txt \
        -ttree ../sample-data/sample-submission-version/TM-training-set/english.tree.txt \
        -aln   ../sample-data/sample-submission-version/TM-training-set/Alignment.txt \
        -out   ../work/model.syntax.t2t/syntax.tree2tree.rule
```

“-model”为机器翻译的模型，该参数决定生成那种类型的规则，具体取值为“s2t”，“t2s”或“t2t”。

“-src”，“-tgt”和“-aln”分别为源语言，目标语言以及二者之间的词对齐文件（每行一句）。

“-stree”为源语言句子的句法树。

“-ttree”为目标语言句子的句法树。

* 输出

串到句法树

输出：在“NiuTrans/work/model.syntax.s2t/”目录下生成3个文件

```
- syntax.string2tree.rule                    # 句法规则表
- syntax.string2tree.rule.bina               # 二叉化句法规则表
- syntax.string2tree.rule.unbina             # 非二叉化句法规则表
```

句法树到串

输出：在“NiuTrans/work/model.syntax.t2s/”目录下生成3个文件

```
- syntax.tree2string.rule                    # 句法规则表
- syntax.tree2string.rule.bina               # 二叉化句法规则表
- syntax.tree2string.rule.unbina             # 非二叉化句法规则表
```

句法树到句法树

输出：在“NiuTrans/work/model.syntax.t2t/”目录下生成3个文件

```
- syntax.tree2tree.rule                      # 句法规则表
- syntax.tree2tree.rule.bina                 # 二叉化句法规则表
- syntax.tree2tree.rule.unbina               # 非二叉化句法规则表
```

* 注意：在运行“NiuTrans-syntax-train-model.pl”前请进入“NiuTrans/scripts/”目录。

## 3.训练N元语言模型

* 操作说明

```
$> cd ../
$> mkdir work/lm/
$> cd scripts/
$> perl NiuTrans-training-ngram-LM.pl \
        -corpus ../sample-data/sample-submission-version/LM-training-set/e.lm.txt \
        -ngram  3 \
        -vocab  ../work/lm/lm.vocab \
        -lmbin  ../work/lm/lm.trie.data
```

“-ngram”为N元语言模型中的元数。如，“-ngram 3”指3元语言模型。

“-vocab”为所生成的目标语端语言模型词汇表。

“-lmbin”为所生成的目标语端语言模型的二进制文件。

* 输出： 生成两个文件，放在目录“NiuTrans/work/lm/”中

```
- lm.vocab                            # 目标语端词汇表
- lm.trie.data                        # 二进制语言模型
```

## 4. 生成配置文件

* 操作说明

串到句法树

```
$> cd NiuTrans/scripts/
$> mkdir ../work/config/ -p
$> perl NiuTrans-syntax-generate-mert-config.pl \
        -model      s2t \
        -syntaxrule ../work/model.syntax.s2t/syntax.string2tree.rule.bina \
        -lmdir      ../work/lm/ \
        -nref       1 \
        -ngram      3 \
        -out        ../work/config/NiuTrans.syntax.s2t.user.config
```

句法树到串

```
$> cd NiuTrans/scripts/
$> mkdir ../work/config/ -p
$> perl NiuTrans-syntax-generate-mert-config.pl \
        -model      t2s \
        -syntaxrule ../work/model.syntax.t2s/syntax.tree2string.rule.bina \
        -lmdir      ../work/lm/ \
        -nref       1 \
        -ngram      3 \
        -out        ../work/config/NiuTrans.syntax.t2s.user.config
```

句法树到句法树

```
$> cd NiuTrans/scripts/
$> mkdir ../work/config/ -p
$> perl NiuTrans-syntax-generate-mert-config.pl \
        -model      t2t \
        -syntaxrule ../work/model.syntax.t2t/syntax.tree2tree.rule.bina \
        -lmdir      ../work/lm/ \
        -nref       1 \
        -ngram      3 \
        -out ../work/config/NiuTrans.syntax.t2t.user.config
```

“-model”表示用于MERT的句法规则类型，具体取值为“s2t”，“t2s”或“t2t”。

“-syntaxrule”为句法规则表。

“-lmdir”为包含N元语言模型和目标语端词汇表的目录。

“-nref”表示源语言句子对应的候选翻译个数。

“-ngram”为N元语言模型中的元数。

“-out”为输出结果，即配置文件。

* 输出

串到句法树

输出：生成配置文件并放在目录“NiuTrans/work/config”下，用户可根据需要修改该文件。

```
- NiuTrans.syntax.s2t.user.config           # 用于MERT和解码的配置文件
```

句法树到串

输出：生成配置文件并放在目录“NiuTrans/work/config”下

```
- NiuTrans.syntax.t2s.user.config           # 用于MERT和解码的配置文件
```

句法树到句法树

输出：生成配置文件并放在目录“NiuTrans/work/config”下

```
- NiuTrans.syntax.t2t.user.config           # 用于MERT和解码的配置文件
```

## 5.权重调优

* 操作说明（要求使用perl）

串到句法树

```
$> cd NiuTrans/scripts/
$> perl NiuTrans-syntax-mert-model.pl \
        -model  s2t \
        -config ../work/config/NiuTrans.syntax.s2t.user.config \
        -dev    ../sample-data/sample-submission-version/Dev-set/Niu.dev.tree.txt \
        -nref   1 \
        -round  2 \
        -log    ../work/syntax-s2t-mert-model.log
```

句法树到串

```
$> cd NiuTrans/scripts/
$> perl NiuTrans-syntax-mert-model.pl \
        -model  t2s
        -config ../work/config/NiuTrans.syntax.t2s.user.config \
        -dev    ../sample-data/sample-submission-version/Dev-set/Niu.dev.tree.txt \
        -nref   1 \
        -round  2 \
        -log    ../work/syntax-t2s-mert-model.log
```

句法树到句法树

```
$> perl NiuTrans-syntax-mert-model.pl \
        -model  t2t \
        -config ../work/config/NiuTrans.syntax.t2t.user.config \
        -dev    ../sample-data/sample-submission-version/Dev-set/Niu.dev.tree.txt \
        -nref   1 \
        -round  2 \
        -log    ../work/syntax-t2t-mert-model.log
```

“-model”为MERT用到的模型类型，具体取值为“s2t”，“t2s”或“t2t”。

“-config”为以前的步骤中生成的配置文件。

“-dev”为权重调优的开发集。

“-nref”为每个源语句子提供的参考译文的数量。

“-round”为执行MERT程序的轮数 (初始设置为1轮 = 10 MERT 循环)。

“-log”为MERT生成的log文件。

* 输出：MERT之后，最优的特征权重被记录在配置文件“-config”的最后一行中。这些特征权重将被用于对新测试句的解码过程中。

## 6. 解码

* 操作说明（要求使用perl）

串到句法树

```
$> cd NiuTrans/scripts/
$> mkdir ../work/syntax.trans.result/ -p
$> perl NiuTrans-syntax-decoder-model.pl \
        -model  s2t \
        -config ../work/config/NiuTrans.syntax.s2t.user.config \
        -test   ../sample-data/sample-submission-version/Test-set/Niu.test.tree.txt \
        -output ../work/syntax.trans.result/Niu.test.syntax.s2t.translated.en.txt
```

句法树到串

```
$> cd NiuTrans/scripts/
$> mkdir ../work/syntax.trans.result/ -p
$> perl NiuTrans-syntax-decoder-model.pl \
        -model  t2s \
        -config ../work/config/NiuTrans.syntax.t2s.user.config \
        -test   ../sample-data/sample-submission-version/Test-set/Niu.test.tree.txt \
        -output ../work/syntax.trans.result/Niu.test.syntax.t2s.translated.en.txt
```

句法树到句法树

```
$> cd NiuTrans/scripts/
$> mkdir ../work/syntax.trans.result/ -p
$> perl NiuTrans-syntax-decoder-model.pl \
        -model  t2t \
        -config ../work/config/NiuTrans.syntax.t2t.user.config \
        -test   ../sample-data/sample-submission-version/Test-set/Niu.test.tree.txt \
        -output ../work/syntax.trans.result/Niu.test.syntax.t2t.translated.en.txt
```

“-model”为解码用到的模型类型，具体取值为“s2t”，“t2s”或“t2t”。

“-config”为配置文件。

“-test”为测试数据集（每行一句）。

“-output”为翻译结果文件（如果未规定此选项，则翻译结果将被放入“stdout”中）。

* 输出

串到句法树

输出：在“NiuTrans/work/syntax.trans.result”目录下生成翻译结果文件

```
- Niu.test.syntax.s2t.translated.en.txt                # 测试句子的1-best翻译结果
```

句法树到串

输出：在“NiuTrans/work/syntax.trans.result”目录下生成翻译结果文件

```
- Niu.test.syntax.t2s.translated.en.txt                # 测试句子的1-best翻译结果
```

句法树到句法树

输出：在“NiuTrans/work/syntax.trans.result”目录下生成翻译结果文件

```
- Niu.test.syntax.t2t.translated.en.txt                # 测试句子的1-best翻译结果
```

## 7. 评价

* 操作说明（要求使用perl）

串到句法树

```
$> perl NiuTrans-generate-xml-for-mteval.pl \
        -1f   ../work/syntax.trans.result/Niu.test.syntax.s2t.translated.en.txt \
        -tf   ../sample-data/sample-submission-version/Reference-for-evaluation/Niu.test.reference \ 
        -rnum 1
$> perl mteval-v13a.pl \
        -r    ref.xml \
        -s    src.xml \
        -t    tst.xml
```

句法树到串

```
$> perl NiuTrans-generate-xml-for-mteval.pl \
        -1f   ../work/syntax.trans.result/Niu.test.syntax.t2s.translated.en.txt \
        -tf   ../sample-data/sample-submission-version/Reference-for-evaluation/Niu.test.reference \
        -rnum 1
$> perl mteval-v13a.pl \
        -r    ref.xml \
        -s    src.xml \
        -t    tst.xml
```

* 句法树到句法树

```
$> perl NiuTrans-generate-xml-for-mteval.pl \
        -1f   ../work/syntax.trans.result/Niu.test.syntax.t2t.translated.en.txt \
        -tf   ../sample-data/sample-submission-version/Reference-for-evaluation/Niu.test.reference \
        -rnum 1
$> perl mteval-v13a.pl \
        -r    ref.xml \
        -s    src.xml \
        -t    tst.xml
```

“-1f”为测试句子集的1-best翻译结果文件。

“-tf”为测试句子集的源语言句子及参考翻译结果文件。

“-rnum”为每个测试句子含有的参考译文。

“-r”为参考翻译文件。

“-s”为源语句的文件。 

“-t”为MT系统生成的（1-best）翻译结果文件。

* 输出：显示IBM版的BLEU值。如果以上各个步骤正确，您将得到测试样本数据的BLEU值约为0.2277、0.2205、0.1939

* 注意：脚本mteval-v13a.pl依赖XML::Parser包。如果您的系统中没有安装XML::Parser，请按照以下命令行进行安装。

```
$> su root
$> tar xzf XML-Parser-2.41.tar.gz
$> cd XML-Parser-2.41/
$> perl Makefile.PL
$> make install
```

# 高级用法

## 配置文件

对高级用户来说，NiuTrans含有很多非常有帮助的特征。所有这些特征都可以通过修改安装包中的配置文件来得以实现。下面将对本系统中使用的配置文件进行描述。

NiuTrans.phrase.user.config
“NiuTrans.phrase.user.config”记录解码器的设置。用户可以修改该文件来使用NiuTrans中更多高级的特征。“NiuTrans.phrase.user.config”包含以下基本特征。

```
###########################################
### NiuTrans decoder configuration file ###
###          phrase-based system        ###
###              2011-07-01             ###
###########################################

#>>> runtime resource tables

# language model
param="Ngram-LanguageModel-File"     value="../sample-data/lm.trie.data"

# target-side vocabulary
param="Target-Vocab-File"            value="../sample-data/lm.vocab"

# MaxEnt-based lexicalized reordering model
param="ME-Reordering-Table"          value="../training/me.reordering.table"

# MSD lexicalized reordering model
param="MSD-Reordering-Model"         value="../training/msd.reordering.table"

# phrase translation model
param="Phrase-Table"                 value="../training/phrase.translation.table"

#>>> runtime parameters

# number of MERT iterations
param="nround"                       value="10"

# order of n-gram language model
param="ngram"                        value="3"

# use punctuation pruning (1) or not (0)
param="usepuncpruning"               value="1"

# use cube-pruning (1) or not (0)
param="usecubepruning"               value="1"

# use maxent reordering model (1) or not (0)
param="use-me-reorder"               value="1"

# use msd reordering model (1) or not (0)
param="use-msd-reorder"              value="1"

# number of threads
param="nthread"                      value="4"

# how many translations are dumped
param="nbest"                        value="20"

# output OOVs and word-deletions in the translation result
param="outputnull"                   value="0"

# beam size (or beam width)
param="beamsize"                     value="20"

# number of references of dev. set
param="nref"                         value="1"

#>>> model parameters

# features defined in the log-linear model
#  0: n-gram language model
#  1: number of target-words
#  2: Pr(e|f). f->e translation probablilty.
#  3: Lex(e|f). f->e lexical weight
#  4: Pr(f|e). e->f translation probablilty.
#  5: Lex(f|e). e->f lexical weight
#  6: number of phrases
#  7: number of bi-lex links (not fired in current version)
#  8: number of NULL-translation (i.e. word deletion)
#  9: MaxEnt-based lexicalized reordering model
# 10: <UNDEFINED>
# 11: MSD reordering model: Previous & Monotonic
# 12: MSD reordering model: Previous & Swap
# 13: MSD reordering model: Previous & Discontinuous
# 14: MSD reordering model: Following & Monotonic
# 15: MSD reordering model: Following & Swap
# 16: MSD reordering model: Following & Discontinuous

# feature weights
param="weights" \
value="1.000 0.500 0.200 0.200 0.200 0.200 0.500 0.500 -0.100 1.000 0.000 0.100 0.100 0.100 0.100 0.100 0.100"

# bound the feature weight in MERT
# e.g. the first number "-3:7" means that the first feature weight ranges in [-3, 7]
param="ranges" \
value="-3:7 -3:3 0:3 0:0.4 0:3 0:0.4 -3:3 -3:3 -3:0 -3:3 0:0 0:3 0:0.3 0:0.3 0:3 0:0.3 0:0.3"

# fix a dimention (1) or not (0)
param="fixedfs"  value="0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0"
```

NiuTrans.phrase.train.model.config
“\config\NiuTrans.phrase.train.model.config”记录训练翻译模型和调序模型的设置，该文件包含以下信息。

```
###########################################
### NiuTrans  phrase train model config ###
###########################################

# temp file path
param="Lexical-Table"                value="lex"
param="Extract-Phrase-Pairs"         value="extractPhrasePairs"

# phrase table parameters
param="Max-Source-Phrase-Size"       value="3"                          # number greater than 0
param="Max-Target-Phrase-Size"       value="5"                          # number greater than 0
param="Phrase-Cut-Off"               value="0"                          # number not less than 0

# phrase translation model
param="Phrase-Table"                 value="phrase.translation.table"

# maxent lexicalized reordering model
param="ME-max-src-phrase-len"        value="3"                          # > 0 or = -1 (unlimited)
param="ME-max-tar-phrase-len"        value="5"                          # > 0 or = -1 (unlimited)
param="ME-null-algn-word-num"        value="1"                          # >= 0 or = -1 (unlimited)
param="ME-use-src-parse-pruning"     value="0"                          # "0" or "1"
param="ME-src-parse-path"            value="/path/to/src-parse/"        # source parses (one parse per line)
param="ME-max-sample-num"            value="5000000"                    # number greater than 0 or "-1" (unlimited)
param="ME-Reordering-Table"          value="me.reordering.table"

# msd lexicalized reordering model
param="MSD-model-type"               value="1"                          # "1", "2" or "3"
param="MSD-filter-method"            value="tran-table"                 # "tran-table" or "msd-sum-1"
param="MSD-max-phrase-len"           value="7"                          # number greater than 0
param="MSD-Reordering-Model"         value="msd.reordering.table"
```

## 实用的功能与技巧

## 如何生成N-BEST翻译结果

It can be trivially done by setting parameter "nbest" defined in "NiuTrans.phrase.user.config". E.g. if you want to generate a list of 50-best translations, you can modify "NiuTrans.phrase.user.config" as follows: 

```
# how many translations are dumped
param="nbest"                     value="50"
```

## 如何扩大Beam宽度

In the NiuTrans system beam width is controlled by the parameter "beamsize" defined in "NiuTrans.phrase.user.config". E.g. if you wish to choose a beam of width 100, you can modify "NiuTrans.phrase.user.config", as follows: 

```
# beam size (or beam width)
param="beamsize"                     value="100"
```

## 采用何种剪枝方法

The current version supports two pruning methods: punctuation pruning and cube pruning. The first method divides the input sentence into several segments according to punctuations (such as comma). The decoding is then performed on each segment individually. Finally the translation is generated by gluing the translations of these segments. The second method can be regarded as an instance of heuristic search. Here we re-implement the method described in (Chiang, 2007).
To activate the two pruning techniques, users can fire the triggers "usepuncpruning" and "usecubepruning" defined in "NiuTrans.phrase.user.config". Of course, each of them can be set individually.

```
# use punctuation pruning (1) or not (0)
param="usepuncpruning"               value="1"

# use cube-pruning (1) or not (0)
param="usecubepruning"               value="1"
```

## 如何加速解码器

A straightforward solution is pruning. As described above, punctuation pruning and/or cube pruning can be employed for system speed-up. By default both of them are activated in our system (On Chinese-English translation tasks, they generally lead to a 10-fold speed improvement). Also, multi-thread running-mode can make the system faster if more than one CPU/core is available. To run the system on multiple threads, users can use the parameter "nthread" defined in "NiuTrans.phrase.user.config". E.g. if you want to run decoder with 6 threads, you can set "nthread" like this

```
# number of threads
param="nthread"                      value="6"
```

To further speed-up the system, another obvious solution is to filter the translation table and the reordering model using input sentences. This feature will be supported in the later version of the system.

## 可使用多少参考译文

The NiuTrans system does not any upper limit on the number of reference translations used in either weight tuning or evaluation. E.g. if you want to use 3 references for weight tuning, you can format your tuning data file as follows (Note that "#" indicates a comment here, and SHOULD NOT appear in users' file).

```
澳洲 重新 开放 驻 马尼拉 大使馆               # sentence-1
                                              # a blank line
australia reopens embassy in manila           # the 1st reference translation
australia reopened manila embassy             # the 2nd reference translation
australia reopens its embassy to manila       # the 3rd reference translation
澳洲 是 与 北韩 有邦交 的 少数 国家 之 .      # sentence-2
```

Then set the "-nref" accordingly. For weight tuning (Note: "-nref 3"),

```
$> perl NiuTrans-mert-model.pl \
        -dev   ../sample-data/sample-submission-version/Dev-set/Niu.dev.txt \
        -c     ../work/NiuTrans.phrase.user.config \
        -nref  1 \
        -r     3 \
        -l     ../work/mert-model.log
```

For evaluation (Note: "-nref 3"),

```
$> perl NiuTrans-generate-xml-for-mteval.pl \
        -1f    1best.out \
        -tf    test-ref.txt \
        -rnum  3
```

## 如何使用高阶的N元语言模型

You first need to choose the order for n-gram language model. E.g. if you prefers a 5-gram languguage model, you can type the following command to train LM (NOTE: "-n 5").

```
$> ../bin/NiuTrans.LMTrainer \
        -t  sample-submission-version/LM-training-set/e.lm.txt -n 5 \
        -v  lm.vocab \
        -m  lm.trie.data
```

Then set the config file accordingly

```
$> cd scripts/
$> perl NiuTrans-generate-mert-config.pl \
        -tmdir  ../work/model/ \
        -lmdir  ../work/lm/ \
        -ngram  5 \
        -o      ../work/NiuTrans.phrase.user.config
```

## 如何控制短语翻译表的大小

To avoid extremely large phrase table, "\config\NiuTrans.phrase.train.model.config" defines two parameters "Max-Source-Phrase-Size" and "Max-Target-Phrase-Size" which control the maximum number of words on source-side and target-side of a phrase-pair, respectively. Generally both two parameters greatly impact the number of resulting phrase-pairs. Note that, although extracting larger phrases can increase the coverage rate of a phrase table, it does not always benefit the BLEU improvement due to data sparseness.
    Another way to reduce the size of phrase table is to throw away the low-frequency phrases. This can be done using the parameter "Phrase-Cut-Off" defined in "\config\NiuTrans.phrase.train.model.config". When "Phrase-Cut-Off" is set to n, all phrases appearing equal to or less than n times are thrown away by the NiuTrans system.
    E.g. the example below shows how to obtain a phrase table with areasonable size. In this setting, the maximum number of source words and target words are set to 3 and 5, respectively. Moreover, all phrases with frequency 1 are filtered.
	
```
param="Max-Source-Phrase-Size"       value="3"
param="Max-Target-Phrase-Size"       value="5"
param="Phrase-Cut-Off"               value="1"
```	
	
## 	如何使用更多语料训练ME调序模型
	
We follow the work of (Xiong et al., 2006) to design the ME-based lexicalized reordering model. In general, the size of the (ME-based) reordering model increases greatly as more training data is involved. Thus several parameters are defined to control the size of the resulting model. They can be found in the configuration file "\config\NiuTrans.phrase.train.model.config", and start with symbol "ME-". 
    1. "ME-max-src-phrase-len" and "ME-max-tar-phrase-len" restrict the maximum number of words appearing in the source-side phrase and target-side phrase. Obviously larger "ME-max-src-phrase-len" (or "ME-max-tar-phrase-len") means a smaller model.
    2. "ME-null-algn-word-num" limits the number of unaligned target words that appear between two adjacent blocks. 
    3. "ME-use-src-parse-pruning" is a trigger, and indicates using source-side parse to constraint the training sample extraction. In our in-house experiments, using source-side parse as constraints can greatly reduce the size of resulting model but does not lose BLEU score significantly.
    4. "ME-src-parse-path" specifies the file of source parses (one parse per line). It is meaningful only when "ME-use-src-parse-pruning" is turned on. 
    5. "ME-max-sample-num" limits the maximum number of extracted samples. Because the ME trainer "maxent(.exe)" cannot work on a very large number of training samples, controlling the maximum number of extracted samples is a reasonable way to avoid the unacceptable training time and memory cost. By default, "ME-max-sample-num" is set to 5000000 in the NiuTrans system. This setting means that only the first 5,000,000 samples affect the model training, and a too large training corpus does not actually result in a larger model.
    To train ME-based reordering model on a larger data set, it is recommended to set the above parameters as follows (for Chinese-to-English translation tasks). Note that this setting requires users to provide the source-side parse trees for pruning.	
	
```
param="ME-max-src-phrase-len"        value="3"
param="ME-max-tar-phrase-len"        value="5"
param="ME-null-algn-word-num"        value="1"
param="ME-use-src-parse-pruning"     value="1"                      # if you have source parses
param="ME-src-parse-path"            value="/path/to/src-parse/"
param="ME-max-sample-num"            value="-1"                     # depends on how large your corpus is
                                                                    # can be set to a positive number as needed
```	
	
## 	如何使用更多语料训练MSD调序模型
	
It is worth pointing out that the NiuTrans system have three models to calculate M, S, D probabilities. Users can choose one of them using the parameter "MSD-model-type". When "MSD-model-type" is set to "1", the MSD reordering is modeled on word-level, as what the Moses system does. In addition to the basic model, the phrase-based MSD model and the hiearachical phrase-based MSD model (Galley et al., 2008) are also implemented. They can be use when "MSD-model-type" is set to "2" or "3".
    When trained on a large corpus, the MSD model might be very large. The situationis even more severe when model 3 is involved. To alleviate this problem, users can use the parameter "MSD-filter-method" which filters the MSD model using phrase translation table (any entry that is not covered by the phrase table are excluded).
    Also, users can use the parameter "MSD-max-phrase-len" to limit the maximum number of words in a source or target phrase. This parameter can effectively limit the size of the generated MSD model. 
    Below gives an example that restricts the MSD to a acceptable size.	

```
param="MSD-model-type"               value="1"                             # "1", "2" or "3"
param="MSD-filter-method"            value="tran-table"                    # "tran-table" or "msd-sum-1"
param="MSD-max-phrase-len"           value="7"                             # number greater than 0
```
	
## 	如何使用自定义特征
	
The NiuTrans system allows users to add self-developed features into the phrase translation table. In the default setting, each entry in the translation table is associated with 6 features. E.g. below is a sample table ("phrase.translation.table"), where each entry is coupled with a 6-dimention feature vector.

```
...
一定 ||| must ||| -2.35374 -2.90407 -1.60161 -2.12482 1 0
一定 ||| a certain ||| -2.83659 -1.07536 -4.97444 -1.90004 1 0
一定 ||| be ||| -4.0444 -5.74325 -2.32375 -4.46486 1 0
一定 ||| be sure ||| -4.21145 -1.3278 -5.75147 -3.32514 1 0
一定 ||| ' ll ||| -5.10527 -5.32301 -8.64566 -4.80402 1 0
...
```

To add new features into the table, users can append them to these feature vectors. E.g. suppose that we wish to add a feature that indicates whether the phrase pair is low-frequency in the training data (appears only once) or not (appears two times or more). We can update the above table, as follows:

```
..
一定 ||| must ||| -2.35374 -2.90407 -1.60161 -2.12482 1 0 0
一定 ||| a certain ||| -2.83659 -1.07536 -4.97444 -1.90004 1 0 0
一定 ||| be ||| -4.0444 -5.74325 -2.32375 -4.46486 1 0 0
一定 ||| be sure ||| -4.21145 -1.3278 -5.75147 -3.32514 1 0 0
一定 ||| ' ll ||| -5.10527 -5.32301 -8.64566 -4.80402 1 0 1
...
```

We then modify the config file "NiuTrans.phrase.user.config" to activate the newly-introduced feature.

```
param="freefeature"                   value="1"
param="tablefeatnum"                  value="7"
```

where "freefeature" is a trigger that activates the use of additional features. "tablefeatnum" sets the number of features defined in the table.

## 如何在解码器中加入额外翻译规则

The NiuTrans system also defines some special markups to support this feature. E.g. below is sample sentence to be decoded.

```
彼得泰勒 是 一名 英国 资深 金融 分析师 .
(Peter Taylor is a senior financial analyst at UK .)
```

If you have prior knowledge about how to translate "彼得泰勒" and "英国", you can add your own translations using the special markups.:

```
彼得泰勒 是 一名 英国 资深 金融 分析师 . |||| {0 ||| 0 ||| Peter Taylor ||| $ne ||| 彼得泰勒} \
{3 ||| 3 ||| UK ||| $ne ||| 英国}
```

where "||||" is a separator, "{0 ||| 0 ||| Peter Taylor ||| $ne ||| 彼得泰勒}" and "{3 ||| 3 ||| UK ||| $ne ||| 英国}" are two user-defined translations. Each consists of 5 terms. The first two numbers indicate the span to be translated; the third term is the translation specified by users; the fourth term indicates the type of translation; and the last term repeats the corresponding word sequence. Note that "\" is used to ease the display here. Please remove "\" in you file, and use "彼得泰勒 是 一名 英国 资深 金融 分析师 . |||| {0 ||| 0 ||| Peter Taylor ||| $ne ||| 彼得泰勒}{3 ||| 3 ||| UK ||| $ne ||| 英国}" directly.










