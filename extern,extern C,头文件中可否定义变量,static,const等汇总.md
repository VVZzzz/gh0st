# extern , extern "C" 作用, 该不该在头文件中定义全局变量? 以及C中的const static,C++中的static 和 const区别联系.

## extern
首先说明extern,一般来说extern的作用是告诉编译器,我要引用别的模块定义的变量或者函数啦. 即extern xxx;只用来声明,而不定义.
一般用法如下: 
对于```extern 变量```:
 ```
 A.c
 #include <stdio.h>
 extern int a;  //声明我要使用其他模块定义的a
 int main() {printf("%d\n",a);}

 B.c
 int a = 10;  //定义a
 ```
即A.c中是的```extern int a ;```告诉编译器,我只声明这个变量```a```,它是其他模块的,我要引用它.而不定义它,你不用为它分配空间.
在连接阶段,即```unix```下是生成```.o```文件,```windows```下生成```.obj```文件.经过符号重定位,链接器将```A.o```中的```a```与```B.o```中的符号```a```关联.成功引用.    
注意这里,只有两个单独的```c```文件.在进行编译时,每个```.c```文件都是单独编译的,生成单独的```.o```文件.之后通过链接器进行符号解析和重定位等生成可执行文件.

对于```extern 函数```: 
 ```
 A.c
 #include <stdio.h>
 extern int a;
 extern void f1();  //声明要使用其他模块定义的f1()
 int main() {printf("%d\n",a); f1();}

 B.c
 int a = 10;
 //定义f1()
 void f1() {printf("this is f1()");}
 ```

 其实对于```变量```还是```函数```,在链接器看来都是```符号```,```extern```的目的就是避免**符号的重定义**.

## .h
再来谈谈```.h```,文件中定义全局变量.在实验时发现一个问题,就是假如我们在```.h```文件中定义一个全局变量.并在多个```c```文件中都包含这个```.h```文件.如下:   
```
C.h
#ifndef __C_H_
#define __C_H_
int a = 10;
#endif

D.h
#include "C.h"

A.c
#include <stdio.h>
#include "C.h"
#include "D.h"
int main() {printf("%d\n",a);}

B.c
#include "C.h"
#include "D.h"
```
即使有头文件保护符,在编译阶段没出错,而在链接阶段出现问题.
```unix/g++: multiple definition of 'a' ```
```windows/msvc: 找到一个或多个重定义符号 ```

我们来分析一下这个```.h```文件看看这个头文件保护符为何不起作用: 
从源文件到可执行文件,要经历```"预处理"->"编译"(高级语言.c->.s汇编语言)->"汇编"(汇编语言.s->.o二进制)->"链接(.o->可执行二进制文件)"```
在```预处理```步骤中:  
1. 将```include```的文件原封不动的替换.即这个时候就不存在```.h```文件了,全是```.c/.cpp```文件.
2. 将宏进行替换.

所以我们发现,经过预处理后```A.cpp```和```B.cpp```文件由于保护符的作用,都是只含有一句```int a = 10 ```.好的,说明头文件保护符确实起了作用.   
但是头文件保护符作用也**仅限于此**.    
至此,编译阶段是没有任何问题的.由于**源文件都是独立编译**的,故也各自得到```.o```文件.即在```A.o```中有符号```a```,存在```.data```中,但而在```B.o```中也有符号```a```,存在```.data```中.    
故到了链接阶段,要符号解析.这两个```a```都是全局的,**强符号**,那么就显然出现了符号重定义.

所以这也是为何我们不能在```.h```文件中定义全局变量的原因.那该如何使用```.h```文件呢? 我们只能在```.h```文件中进行声明,也就是```extern int a;```然后在相应的一个```.cpp```文件中进行定义```int a;```,其他模块想用这个变量的话,就包含这个```.h```文件即可.如下:  
```
正确 √
C.h 
#ifndef __C_H_
#define __C_H_
extern int a;  //声明
#endif

A.cpp
#include "C.h"
//这个C.h就相当于 extern int a;这句话
int main() {cout<<a;}

B.cpp
#include C.h //包不包含都不影响
int a = 10;  //定义.
```

同样,```.h```文件也不能定义函数,否则同样会出现符号重定义.如下:
```
错误 ❌
C.h 
#ifndef __C_H_
#define __C_H_
void f1() {}  //定义了f1()函数符号.
#endif

A.cpp
#include "C.h"
int main() {return 0;}

B.cpp
#include C.h 
```
在链接阶段:报错,**符号重定义**.
正常使用```.h```文件: 在头文件中只声明这个函数,而在对应的源文件中去定义.而其他源文件要想使用这个函数,包括头文件即可.如下:  
```
正确 √
C.h 
#ifndef __C_H_
#define __C_H_
extern void f1();  //声明f1()函数符号.extern 这里加不加无影响.
#endif

A.cpp
#include "C.h"
int main() { f1(); return 0;}

B.cpp
#include C.h 
void f1() {}  //定义f1()函数符号
```

但是注意有几个例外的情况,是可以在```.h```文件中定义的: 
- 在编译期间就知道值的const变量,可以在头文件中定义.
 ```
 编译链接正确 √
 C.h
 #ifndef __C_H_
 #define __C_H_
 const int a = 10;  //并且在定义时就要初始化.
 #endif

 A.cpp
 #include "C.h"
 int main() {cout<<a;}

 B.cpp
 #include "C.h"
 ```
 这里我们就发现和上面只有一个地方不同,即```const int 和 int```,那么为什么这个链接阶段就没有出现问题呢?  我们稍候分析!
- 类的定义可以放在```.h```中,这种情况比较常见.
- ```inline```函数定义在```.h```中,这种情况也比较常见.

## ```extern "C"```
在c++的程序中,假如我们想使用一段C编写的代码或者是其他语言编写的代码,要使用另外一个编译器进行编译链接某段特定的代码时,我们该怎么做呢?   

这时,我们就使用```extern "C"```,这个```"C"```,表示的一种编译和连接规约，而不是一种语言。C表示符合C语言的编译和连接规约的任何语言，如```Fortran```、```assembler```等.

- 注意,extern "C"指令仅指定编译和连接规约，但不影响语义。例如在函数声明中，指定了extern "C"，仍然要遵守C++的类型检测、参数转换规则.
- 在C++源文件中的语句前面加上extern "C"，表明它按照类C的编译和连接规约来编译和连接，而不是C++的编译的连接规约。这样在类C的代码中就可以调用C++的函数or变量等.(注：我在这里所说的类C，代表的是跟C语言的编译和连接方式一致的所有语言).
- 我们既然知道extern "C"是实现的类C和C++的混合编程。下面我们就分别介绍如何在C++中调用C的代码、C中调用C++的代码。首先要明白C和C++互相调用，你得知道它们之间的编译和连接差异，及如何利用extern "C"来实现相互调用。 

1. C++的编译链接
C++是一个面向对象语言（虽不是纯粹的面向对象语言），它支持函数的重载，重载这个特性给我们带来了很大的便利。为了支持函数重载的这个特性，C++编译器实际上将下面这些重载函数：
    ```
    void print(int i);
    void print(char c);
    void print(float f);
    void print(char* s);
    ```
    编译为:
    ```
    _print_int
    _print_char
    _print_float
    _pirnt_string
    ```
    这样的函数名，来唯一标识每个函数。注：不同的编译器实现可能不一样，但是都是利用这种机制。所以当连接是调用```print(3)```时，它会去查找```_print_int(3)```这样的函数。下面说个题外话，正是因为这点，**重载被认为不是多态，多态是运行时动态绑定（“一种接口多种实现”），如果硬要认为重载是多态，它顶多是编译时“多态”。**
    C++中的变量，编译也类似，如全局变量可能编译g_xx，类变量编译为c_xx等。连接是也是按照这种机制去查找相应的变量。

2. C的编译链接
C语言中并没有重载和类这些特性，故并不像C++那样```print(int i)```，会被编译为```_print_int```，而是直接编译为```_print```等。因此如果直接在C++中调用C的函数会失败，因为连接是调用```print(3)```,链接器会去找```_print_int(3)```。而不会去链接```_print```,因此```extern "C"```的作用就体现出来了。
3. C++中调用C代码
    示例如下:
    ```
    cfunc.h
    #ifndef __CFUNC_H
    #define __CFUNC_H
    void myprint(int i);
    #endif

    cfunc.c
    #include "cfunc.h"
    void myprint(int i) {printf("cheader %d",i);}

    main.cpp
    #include <stdio.h>
    //或者extern "C" void myprint(int i);不包含头文件
    extern "C" {
        #include "cfunc.h"
    }
    int main() {
        myprint(2);
        return 0;
    }
    ```
4. C中调用C++代码
C中调用C++代码,有略微不同,如下:
    ```
    cppfunc.h
    #ifndef __CPPFUNC_H
    #define __CPPFUNC_H
    #include <iostream>
    extern "C" void myprint(int i);
    #endif

    cppfunc.cpp
    #include "cppfunc.h"
    void myprint(int i) {std::cout<<"C++ "<<i;}

    main.c
    #include <stdio.h>
    //错误❌用法: extern "C" void myprint(int i); 编译不通过,"C"应输入标识符
    //错误❌用法: #inlcude cppfunc.h , 和上句一样.
    //正确用法: extern void myprint(int i);
    int main() {
        myprint(2);
        return 0;
    }
    ```
    5. gcc和g++
    对于```3```和```4```中,我们可能会疑惑使用gcc还是g++编译链接呢?   
    这二者区别并不大:   
    - gcc将编译：* .c / * .cpp文件分别看待为C和C ++.
    - g ++将编译：* .c / * .cpp文件，但它们都将被视为C ++文件.
    - 此外，如果您使用g ++链接目标文件，它会自动链接到std C ++库（gcc不会这样做）.
    - gcc compiling *.cpp and g++ compiling *.c/*.cpp files has a few extra macros.
    总结来说:
    g++约等于```gcc -xc++ -lstdc++ -shared-libgcc```（第一个是编译器选项，第二个是链接器选项）。 这可以通过使用-v选项运行来检查（它显示正在运行的后端工具链命令）。
    这里只是约等,因为gcc -lstdc++没有对```math,RTTI,expection information```等的支持.  
    故我们对于c++程序,直接用g++编译链接即可.如果是混合编程,就要注意链接时要关联stdc++的库.
    ```参考回答: https://stackoverflow.com/questions/172587/what-is-the-difference-between-g-and-gcc/172592#172592```

## C和C++中的const static
1. C中的```static```关键字
在探讨C中的```static```关键字中,有必要了解一下变量在内存存放的区域.   
一般来说可以内存结构如下:   
    ```
    内核栈
    栈↓
    .
    .
    共享库代码
    .
    .
    堆↑
    .data(已初始化数据段) .bss(未初始化数据段)
    .text(代码段)
    保留区
    ```
    - 局部变量,函数参数等都存放在栈区.
    - 用户开辟(malloc new)的变量存放在堆区.
    - static变量(无论是否是局部还是全局), 全局变量都存放在.data和.bss中即数据区中.    

所以```static```关键字作用其一:  修饰的**变量**,生命周期一直到程序结束.无论这个变量是**局部**的还是**全局**的.如下:
```
void fun() {
    static int i = 10;
    i++;
    printf("%d\n",i);
}
```
第一次调用```fun()```时,定义并初始化了一个static**局部**变量i,调用结束后.因为这个**局部变量**没有存放在栈中,故这个i不会**消亡**,而是一直存放在```.data```中. 下一次再调用```func()```时,就不会再定义初始化了,因为已经有i了,所以第二次调用会输出```12```.

在这里注意,在数据区.data .bss中,不存在符号重定义.即看如下例子:
```
static int i = 5;

void f1() {
  static int i = 2;
  printf("f1: %d\n", ++i);
}

void f2() {
  static int i = 3;
  printf("f2: %d\n", ++i);
}

int main(int argc, char **argv) {
  f1();
  f2();
  f1();
  f2();
  printf("%d\n", i);
  //fA();
  return 0;
}
```
虽然定义了3个都叫```i```的```static```变量,但在.data段中,是不同的符号,即```i.1```,```i.2```,```i.3```.故上述不存在重定义的问题.输出:  
```
f1: 3
f2: 4
f1: 4
f2: 5
5
```

```static```关键字作用其二:  隔离作用域.   
```static```修饰后的符号,都只能在当前模块定义并引用.其他模块无法访问引用.包括**变量和函数**.即:  
```
A.cpp
static int i = 10;  //全局static变量,但只在A.cpp中可见
static void fA() {};

B.cpp
extern int i;  //错误,无法解析符号
extern void fA();  //错误,无法解析符号
```
所以,这实际上提供了保护功能.即两个文件如果有同名的函数,变量.也由于作用域隔离互不影响,因为只能在本文件内可见.

2. C语言中的const:  
C中的```const```修饰的作用,只有一点.即使这个变量的值,在定义时确定,在整个生命周期中不能更改.    
故```const ```变量在定义时必须初始化,比如我们想要一个值为5的int常量.    
```
const int i = 5;  //正确

const int i;
i = 5;  //错误,不能更改常量的值.
//实际上const int i;已经对i默认初始化了.
```
注意它与static的区别,const 修饰局部变量,那么这个变量仍然放在**栈**区中,而不是static修饰局部变量的效果.当然const 修饰全局变量的话,那这个变量就放在.data段中.

3. C++语言中的static和const
C++中的static和const继承C语言中的特性外,最重要的是它们在```类```这个C中没有的东西上面的用法.
故详情再看[这篇博客](https://www.runoob.com/w3cnote/cpp-static-const.html),c++中的static和const.

