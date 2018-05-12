运行run-generate.bat可以生成评价用的1best.xml、ref.xml和src.xml文件
运行之前需要设置一下参数

set task=en_zh_news_trans（英汉任务）

set translist=1best.dev.round1（系统输出翻译结果文件）

set reflist=ref.dev.E2C.txt（翻译参考答案文件，dev格式）

set nref=4（答案个数）

