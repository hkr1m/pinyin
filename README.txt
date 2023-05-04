一、文件结构
1. src -- 源文件目录，包含
pinyin.cpp -- 源程序，包含二元模型类 BinaryGrammerModel 和多元模型类 MultipleGrammerModel，默认使用二元模型
getfreq.cpp -- 从语料库获取词频表
compare.cpp -- 比较输出和标准输出
alphabet.txt -- 常用汉字表
pinyin.txt -- 拼音汉字表
word_freq2.txt -- 二元词频表
word_freq3.txt -- 三元词频表
(word_freq4.txt) -- 四元词频表，位于清华云盘 https://cloud.tsinghua.edu.cn/f/79add152b5db4e2e84cb/

2. bin -- 可执行文件目录，包含
pinyin / pinyin.exe -- 基于字的语法模型，可使用二元、三元和四元模型
getfreq / getfreq.exe -- 测试环境下从语料库中获取词频表
compare / compare.exe -- 测试环境下比较输出和标准输出

二、程序运行方式（bin 目录下）
Mac:
./pinyin <input_path> <output_path> [model] [max_word]
Windows:
.\pinyin <input_path> <output_path> [model] [max_word]

必选参数 input_path 和 output_path 表示输入文件和输出文件路径。
可选参数 model 为使用模型类型，为整数 N = 2, 3 或 4，表示使用 N 元模型。默认使用二元模型。
可选参数 max_word 为非负整数时表示最多从词频表中读入的词数，为负整数时表示无限制。默认无读入词数限制。

./getfreq <output_path> [length] [max_word]
从 ../src/sina_news_gbk 中的新浪新闻语料库读入，将长度不超过 length(默认 = 2) 的子串和词频以格式 (子串, 出现次数) 输出到 output_path 中，限制只输出出现次数最多的 max_word 个子串（默认无限制）。

./compare <output_path> <std_output_path>
比较 output_path 和 std_output_path 两个文件，输出字准确率和句准确率。仅支持两个文件同为 gbk 编码的情况。


例:
./pinyin ../data/input.txt ../data/output.txt 3

三、注意事项
最终程序的多元模型应用了实验报告中的词频表优化和 epsilon 剪枝，一定程度上平衡了正确率与时间内存消耗。

若使用多元模型 (N=3, 4) 时 Building Model 耗时过长（2min 以上无结果），可以使用 max_word 参数限制最多从词频表中读入的词数。N=3 时在 10^7 以下调整，N=4 时在 3*10^7 以下调整，牺牲正确率来平衡效率。

使用四元模型时，从 https://cloud.tsinghua.edu.cn/f/79add152b5db4e2e84cb/?dl=1 下载 word_freq4.txt 置于 src 目录下再运行 pinyin。四元模型正确率较三元模型提升不多，且效率一般，可选择性调用。
