# WebBrowser

## 设计思路
1. 每个新的webbrowser控件均在单独的一个线程中运行，
api通过::PostThreadMessage与该线程交互。
2. 提供js代码注入功能
3. 提供注册额外的cpp函数，供页面调用

## API文档详情参考`Browser/MyBrowser.h`
