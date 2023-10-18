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
wget

ByteStream


lab1:

Reassembler


lab2:
wrap     绝对序号到相对序号
unwrap   相对序号到绝对序号

TCPreceiver
碰到问题： winddowsize是uint16的，而Bytestream容量是uint64的，计算窗口大小需要考虑比较。
       ：碰到一个编译问题 （is implicitly deleted because the default definition would be ill-formed:）。可能和编译器有关，暂时不知道为什么。


