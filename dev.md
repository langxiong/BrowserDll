# 开发思路
## 需求
* 1、支持把主程序里面的任意类或者函数，注册到动态库里面供浏览器控件里的JS调用。
   就是说动态库要提供接口函数来实现注册功能，让程序员很方便的把类和函数变成JS可以调用的接口。
   MFC已经实现了这个机制（JS调用C++），但是都是静态的并且绑定在IE控件上面。按照MFC的机制，如果增加一个接口，必须要修改程序。
   这里的要求是主程序可以修改（方便、简单，并且和MFC无关），但是动态库不能修改。

* 2、支持主程序调用任意的JS函数。即C++调用JS

* 3、主程序可能不是用MFC开发的，并且动态库是动态调用的。即使用 LoadLibrary 函数初始化的。
   请给出解决方案，并定义动态库的接口函数，包括：函数名、参数、返回值、功能说明。

```
分析需求，主要要提供一个Browser.dll，内部导出API接口，供第三方程序调用，并实现c++与js互相调用功能.
```

## 关键点
* 开发时间：因考虑到开发周期只有3天，任务规划时，分析注册任意cpp函数给js调用，会有难点。则该任务最后实现。然后，尽量在保持Browser.dll干净的环境下，移植过去代码，第一天必须完成基础工程搭建，第二天完成大多数需求，并能通过演示demo测试。最后再实现注册任意cpp函数给js调用，如时间紧急，可考虑先实现最简单的普通函数注册功能。
* 动态加载： 针对这点，考虑动态库使用extern "C"导出调用API，并在约定API入参和返回值时，使用基础类型，不引入C++类型，尽量兼容更多系统
* 内部封装浏览器控件：因内部封装的是浏览器控件，考虑到浏览器控件在导航页面时，容易发生假死等行为。封装浏览器控件因将控件运行的线程与调度线程分离，使浏览器控件在另外的线程消息循环中运行
* 嵌入多个浏览器控件：考虑经常会出现需要在客户端中嵌入多个浏览器控件，如登陆、广告窗口等，所以封装的dll应该考虑可以创建多个浏览器控件。又因动态加载dll，考虑到兼容性，所有导出的api（除开创建）均默认第一个入参为来自创建浏览器控件得到的索引index
* 多线程控制： 因浏览器控件运行在Browser.dll内部的线程中，对于调度进程来说是可以透明的，则提供的销毁控件的api需要控制浏览器线程平滑退出。也因考虑多个控件同时退出的速度（程序关闭），因提供一个统一退出的api(TODO，本期未实现)
* cpp调用js: 考虑到需要实现cpp调用js，提供演示demo时，提供一个richedit区域，用于动态输入js代码，方便测试
* js调用cpp: 因需要实现动态注册cpp方法给js调用，提供演示demo时，增加一个list列表(显示已经内置的部分cpp测试函数)，可以通过动态增删，结合cpp调用js代码（js代码中回掉cpp)快速测试代码注册功能.(TODO: 本期演示功能未实现list列表)
* 注册任意cpp方法给js调用：主要考虑到代码的兼容性，cpp代码在每个编译器版本中的实现是有差别的，为达到最好的兼容性，可以约定cpp代码使用基础类型，或者com中的一些基础类型。后调研时，确定js调用cpp，可以采用一个固定的函数签名式，即void pFun(DISPPARAMS *params, VARIANT *result)。考虑到需要兼容普通的注册函数、和cpp成员函数，后调研中发现std::function可以将签名式符合的普通函数和类的成员函数，均转为std::function的对象，所以，在注册cpp给js的接口内部数据存储中，使用std::function作为基础类型的成员(TODO: 本期未验证注册类的成员函数能否转换为std::function的对象)
* WebBrowser.dll：为考虑减少对第三方库的依赖，本dll尽量减少依赖与第三方库，如有依赖，应静态链接第三方库
* 特殊需求，创建的webbrowser控件需要隐藏的处理：如在登陆业务的实现时，常使用cpp调用页面js实现，此时嵌入的浏览器控件因隐藏

## 代码实现的亮点.
* cast_to_function:
> 在做导入dll的函数时，抽离一个方便个人开发的泛型辅助函数，将::GetProcAddress返回的函数转换为std::fucntion

```cpp
template<typename Signature>
std::function<Signature> cast_to_function(FARPROC f)
{
    return std::function<Signature>(reinterpret_cast<Signature*>(f));
}
```
> sample

```cpp
FARPROC pFunc = ::GetProcAddress(m_hBrowserModule, "CreateBrowserCtrl");
if (pFunc)
{
    m_spBrowserModuleApis->_createBrowserCtrl = cast_to_function<int(HWND hBindWnd)>(pFunc);
}
```