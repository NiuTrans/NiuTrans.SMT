# NiuTrans.SMT统计机器翻译系统

* NiuTrans.SMT是一个开源的统计机器翻译系统，由东北大学自然语言处理实验室和沈阳雅译网络技术有限公司联合开发。NiuTrans系统全部由C++开发，运行速度快，占用内存少。目前该系统支持基于（层次）短语/基于句法的模型，可供研究之用。


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
  
## 系统运行要求
* Windows用户：要求Visual Studio 2008，Cygwin，以及 perl（5.10.0 版本或更高）。建议默认在“C:\”路径下安装cygwin。

* Linux用户：要求gcc（4.1.2版本或更高），g++（4.1.2版本或更高），GNU Make(3.81版本或更高)以及perl（5.8.8 版本或更高）。

注意：运行本系统的最低要求为`2GB内存、10GB硬盘空间`。 如果使用大规模的语料训练本系统，则需要更多的内存和硬盘空间。建议使用64位操作系统以支持大规模语料/模型（如N-元语言模型） 

## 安装

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

## NiuTrans.SMT团队
* [朱靖波](http://www.nlplab.com/members/zhujingbo.html)（项目负责人）
* [肖桐](http://www.nlplab.com/members/xiaotong.html)（技术负责人）
* 李垠桥
* 杜权
* 王强
* 姜雨帆
* 林野
* 张裕浩

致谢：在本项目实施的过程中，获得了往届毕业生的支持，他们是：李强（基于短语的系统和各种脚本），张浩（解码器，调序模型），陈如山（语言模型），姚树杰（数据选取和数据预处理），马骥（语言模型，CWMT2013基线系统），孙坤杰（CWMT2013汉英基线系统），刘卓（CWMT2013英汉基线系统）。

## 如何引用NiuTrans
如果您在研究中使用了NiuTrans，请在参考文献中注明引用如下论文

**Tong Xiao, Jingbo Zhu, Hao Zhang and Qiang Li. 2012. NiuTrans: An Open Source Toolkit for Phrase-based and Syntax-based Machine Translation. In Proc. of ACL, demonstration session.**

## 获得帮助
有关NiuTrans, 请发送e-mail至niutrans@mail.neu.edu.cn。我们将为您详细解答。

## NiuTrans历史
* NiuTrans 1.4.0版 - 2018年5月12日（bug修复）
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
