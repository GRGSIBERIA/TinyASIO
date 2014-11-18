---
layout: default
title: Tiny ASIO project page
---
### 概要
TinyASIOはASIO SDKのラッパーライブラリです．
ASIO SDKでの開発を省力化するために作られました．
音声入力を音声出力に流すだけのコードがたったの5行で書くことができます．

```cpp
#include <iostream>
#include "TinyASIO.hpp"

int main() {
  // ライブラリの初期化
  // CoInitializeEx(0, COINIT_MULTITHREADED); // 必要に応じてCOMの初期化を行う
  asio::Driver::Init("AudioBox");  // レジストリに登録されているドライバ名を指定

  auto controller = new asio::InputBackController();  // コントローラの作成
  controller.Start();   // バッファリング開始
  while (true)
  {
    auto fetchedData = controller.Fetch();  // バッファに溜まった音声データを取得する
  }
  controller.Stop();    // バッファリング停止
  return 0;
}
```

TinyASIOは簡単に書けるだけでなく，簡単に拡張することができます．