### ```CBuffer```类
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

### ```ClientSocket```类
1. ```ClientSocket```中:
   ```connect```方法: 有一句开启服务,但开启服务管理器的句柄为NULL.```StartService(NULL, NULL, NULL)``` ?
2. ```connect```方法: 开启TCP中的```keepAlive```机制
3. 析构函数还有```Disconnect()```都中出现这段代码:

        SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
        ...
        CloseServiceHandle(hSCM);
    不知道有何作用?
4. ```Disconnect```方法: 里面有一个```LINGER```结构,设置为```1,0```.
        TCP关闭```socket```时,用以下语句:

        setsockopt(m_Socket, SOL_SOCKET, SO_LINGER, (char *)&lingerStruct, sizeof(lingerStruct));
        CancleIo((HANDLE)m_Socket);
    那么什么是```linger```,TCP关闭```socket```连接时,```linger```又起到了什么作用?
    [博客详解](https://segmentfault.com/a/1190000012345710?utm_source=tag-newest)
    ```CancleIo((HANDLE)m_Socket)```作用


### ```util```模块
该模块是将```CreateThread```函数又重新包装了一遍.
1. 在```MyCreateThread```函数中,使用了一个事件内核对象,```hEventTransferArg```它用来确保线程一定开启,且一定运行线程函数.否则就一直等待```WaitForSingleObject(hEventTransferArg,INFITE)```
            

### ```KeybordManager```模块
1. ```MyFuncInitializition()```方法: ```GetModuleHandle()```将```dll```文件,映射到当前进程的地址空间,并获得其基地址.然后再用```GetProcAddress(module,name)```获得这个```dll```文件中```name```函数的地址,也就是函数指针了.这样就可以拿```dll```文件中的函数过来用.   
但这里有一个问题,为何不直接调用```dll```文件的函数? 或者说```dll```文件中的函数是如何使用的?   
<<核心编程>>第19 20章.