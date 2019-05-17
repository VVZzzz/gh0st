1. ```CBuffer```类中,构造函数初始化临界区```InitializeCriticalSection(&m_cs)```,析构函数释放临界区```DeleteCriticalSection(&m_cs)```.
有关```windows```下临界区请看[这里](https://docs.microsoft.com/zh-cn/windows/desktop/api/synchapi/nf-synchapi-initializecriticalsection).
详解请看<< Windows核心编程 >>
2. ```CBuffer```类中,析构函数释放虚拟内存,即调用了这个```VirtualFree()```函数,请看这个[解析](https://docs.microsoft.com/en-us/windows/desktop/api/memoryapi/nf-memoryapi-virtualfree),还有[例子](https://docs.microsoft.com/zh-cn/windows/desktop/Memory/reserving-and-committing-memory)
3. ```Windows```中的内存映像文件是什么? 