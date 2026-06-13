# worklog-cli

C言語学習用の作業ログ集計CLIアプリです。

日付、カテゴリ、作業内容、作業時間を入力して `log.csv` に保存し、保存済みログの全件表示、日付別表示、カテゴリ別集計ができます。

## 機能

- 作業ログの追加
- 全ログ表示
- 日付別のログ表示
- カテゴリ別の作業時間集計
- 作業時間の合計表示
- Excelで開きやすいUTF-8 BOM付きCSV保存

## ビルド方法

```sh
gcc main.c -o worklog
```

Windowsでは次のように実行ファイル名を指定できます。

```sh
gcc main.c -o worklog.exe
```

## 実行方法

```sh
./worklog
```

Windowsの場合:

```powershell
.\worklog.exe
```

## CSV形式

ログは `log.csv` に保存されます。

```csv
date,category,content,minutes
2026-06-13,C言語,ファイル読み書きの練習,45
```

カテゴリや作業内容にカンマが入るケースには対応していません。

## 学習ポイント

- `printf` による画面表示
- `fgets` による文字入力
- `struct` によるデータ管理
- 固定長配列による複数ログ管理
- `fopen` / `fprintf` / `fgets` / `fclose` によるファイル読み書き
- `strcmp` による文字列比較
- ループ処理
- 条件分岐
- 関数分割
