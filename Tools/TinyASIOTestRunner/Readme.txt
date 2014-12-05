アプリケーション名：TinyASIO Test Runner
作成者：竹渕瑛一（GRGSIBERIA）
連絡先：nanashi4129@gmail.com
Twitter: https://twitter.com/grgsiberia
Blog: http://grgsiberia-mitou.hatenadiary.jp/

TinyASIOが動くかどうか確認するためのテスターです．
導入する前にそもそも動くかどうか確認するために使ってください．

１．
起動するとインストールされているASIOドライバーが表示されます．

２．
次にThreadingModelの選択ですが，
各種ドライバは基本的にApartmentがデフォルトです．
なのですが，ここでは「「「「Both」」」」を選択してください（重要）．

３．
その次は入力チャンネルの選択です．
チャンネル数が多いと下に突き抜けてしまいますが，あまり気にしないでください．

４．
次は出力チャンネルの選択です．

５．
ここでやっと波形が表示されるようになります．
適切な入出力チャンネルが選択されていれば，波形が表示されるはずです！


※他のアプリが動かなくなった時
ThreadingModelを変更したせいで動かなくなっています．
「「「「Apartment」」」」に戻してあげてください．