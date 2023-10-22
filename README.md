Stanford CS 144 Networking Lab
==============================

These labs are open to the public under the (friendly) request that to
preserve their value as a teaching tool, solutions not be posted
publicly by anybody.

Website: https://cs144.stanford.edu

To set up the build system: `cmake -S . -B build`

To compile: `cmake --build build`

To run tests: `cmake --build build --target test`

To run speed benchmarks: `cmake --build build --target speed`

To run clang-tidy (which suggests improvements): `cmake --build build --target tidy`

To format code: `cmake --build build --target format`


lab0:
写一个函数，获取一个网页
wget。是一个获取http协议网页的函数。

设计字节流类。
ByteStream，是TCP协议的有序字节流，实现读写与容量控制。
本次采用STL的deque数据结构存储，按照字节保存与输出。
在peek函数中出现了string_view,string_view对字符串操作不会涉及多余的拷贝，只保存指针与长度。


lab1:
对接收到的TCP数据包进行排序，实现流重组器类。
Reassembler，由于TCP数据不一定是按序到达的，因此需要该类对收到数据排序，在第一个被确认字节到达时就要写入ByteStream。
本次采用两个STL的deque数据结构存储，一个存储到达的数据，一个判断数据否已经存在。本次实现与书中有所不同，书中对到达的不是确认号的数据直接丢弃，本次会直接写入队列。


lab2:
wrap     绝对序号到相对序号
unwrap   相对序号到绝对序号

TCPreceiver
碰到问题： winddowsize是uint16的，而Bytestream容量是uint64的，计算窗口大小需要考虑比较。
       ：碰到一个编译问题 （is implicitly deleted because the default definition would be ill-formed:）。可能和编译器有关，暂时不知道为什么。


