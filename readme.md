1. ```CBuffer```类中,构造函数初始化临界区```InitializeCriticalSection(&m_cs)```,析构函数释放临界区```DeleteCriticalSection(&m_cs)```.
有关```windows```下临界区请看[这里](https://docs.microsoft.com/zh-cn/windows/desktop/api/synchapi/nf-synchapi-initializecriticalsection).
详解请看<< Windows核心编程 >>
2. ```CBuffer```类中,析构函数释放虚拟内存,即调用了这个```VirtualFree()```函数,请看这个[解析](https://docs.microsoft.com/en-us/windows/desktop/api/memoryapi/nf-memoryapi-virtualfree),还有[例子](https://docs.microsoft.com/zh-cn/windows/desktop/Memory/reserving-and-committing-memory)
3. ```Windows```中的内存映像文件是什么? 
4. ```CBuffer```中为何一直要
    ```CKeyboardManager::MyGetModuleFileName(NULL, szModule, MAX_PATH);```
    ```CKeyboardManager::MyGetShortPathName(szModule, szModule, MAX_PATH);```
    ```DeleteFile(szModule);```        
5. ```Buffer类```
这个类有几点值得注意的地方:  
- 用```CRITICAL_SECTION```即临界区,去保护这个```buffer```.同一时刻,只能有1个线程去读,写,访问这个```buffer```.
- 由于可能有多个**进程**,故```buffer```中的分配内存等都是分配的虚拟内存即虚拟地址空间.即用```VirtualAlloc()```和```VirtualFree()```.
6. ```ClientSocket```中:
   ```connect```方法: 有一句开启服务,但开启服务管理器的句柄为NULL.```StartService(NULL, NULL, NULL)``` ?
            