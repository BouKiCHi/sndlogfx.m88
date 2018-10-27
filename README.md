# sndlogfx.m88

これは、音源ログを記録するM88用プラグインです。
VS2017でビルドできるようにしました。

タイミングがおかしくなったり、リセット時の挙動が怪しくなったりしていたので、
コードはほとんど書き換えました。

怪しかった連続録音機能を削除し、最初のキーオン時まで同期をしないという機能も追加しています。
(PSGが最初の場合はうまく動かないと思います。いろいろあります）

コンパイルには親ディレクトリにm88のソースコードが必要になります。
ヘッダファイルと、instthnk.cppに依存しているためです。

これは昔、勝手に作成してどこかに貼り付けたものだと思うので、
個人で楽しむ範囲でどうぞ。
