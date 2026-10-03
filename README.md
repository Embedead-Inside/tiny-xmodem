# tiny-xmodem

[![License: MIT-0](https://img.shields.io/badge/License-MIT--0-blue.svg)](https://spdx.org/licenses/MIT-0.html)
[![Language: C99](https://img.shields.io/badge/Language-C99-green.svg)](#)

ベアメタル環境（組込みC言語）向けの、極めて小さくコンパクトな **XMODEM送受信ライブラリ** です。

ハードウェア依存部（UARTの送受信）を完全に切り離しているため、任意のマイコンアーキテクチャや独自BSP/HAL環境にそのまま組み込んで利用できます。

## 特徴

* **コンパクト＆シンプル**: 128バイトブロックの単純チェックサム方式（Classic XMODEM）に絞り込み、ROM/RAMの消費を最小限に抑えています。
* **bool不使用**: 標準の `bool` 型や複雑な関数テーブル、動的メモリ確保（`malloc`）を一切使用せず、純粋なC言語（C99以降）で記述されています。
* **コールバック統合設計**: 状態変化やデータ送受信は固定名称の共通コールバック関数（`xmodem_callback`）1つで完結します。
* **送受信対応**: 受信だけでなく送信機能も1つのペア（`xmodem.h` / `xmodem.c`）に統合されています。

---

## ファイル構成

* `xmodem.h`: ヘッダーファイル（API、定数定義、依存I/O関数の宣言）
* `xmodem.c`: 送受信のコアロジック実装

---

## 依存する外部I/O関数

利用するプラットフォーム側で、以下の2つの関数を実装する必要があります（通常はUARTのドライバ等でラップします）。

```c
/**
 * @brief 1バイトのUART送信
 * @param b 送信データ
 */
void uart_send(uint8_t b);

/**
 * @brief 1バイトのUART受信（タイムアウト付き）
 * @param b 受信格納先ポインタ
 * @param timeout_ms タイムアウト時間（ミリ秒）
 * @return 1: 成功, 0: タイムアウト/エラー
 */
int uart_recv(uint8_t *b, uint32_t timeout_ms);

```

---

## 使い方・実装例

### 1. 共通コールバックの実装

送受信時のイベントやデータのやり取りを処理する `xmodem_callback` を実装します。

```c
#include "xmodem.h"

int xmodem_callback(uint8_t event, uint8_t seq, uint8_t *data)
{
    switch (event) {
        case XMODEM_START:
            /* セッション開始時の初期化処理（LED点灯、フラッシュ消去準備など） */
            break;

        case XMODEM_PACKET:
            /* 
             * 受信時: data が受信した128バイトデータ
             * 送信時: data に次の128バイトデータを書き込む
             */
            #ifdef IS_RECEIVER
            // flash_write(seq, data); // 例: フラッシュへ書き込み
            #else
            // if (get_data_from_memory(seq, data) != 0) return -1; // データナシなら非0で終了
            #endif
            break;
    }
    return 0; /* 0: 継続, 非0: 中断（XMODEM_CANCELを返して終了） */
}

```

### 2. 受信（ファームウェアアップデートの受け取り等）の呼び出し

```c
void start_receive_firmware(void)
{
    int result = xmodem_receive();

    if (result == XMODEM_COMPLETE) {
        /* 受信成功 */
    } else if (result == XMODEM_CANCEL) {
        /* キャンセルされた */
    } else {
        /* エラー（タイムアウト・チェックサム不一致など） */
    }
}

```

### 3. 送信（データの転送等）の呼び出し

```c
void start_send_data(void)
{
    int result = xmodem_transmit();

    if (result == XMODEM_COMPLETE) {
        /* 送信成功 */
    } else {
        /* エラー */
    }
}

```

---

## ライセンス

このプロジェクトはMIT-0ライセンスのもとで公開されています。詳細は[LICENSE](LICENSE)ファイルをご覧ください。
- 商用利用・個人利用を問わず、完全自由に使用できます。
- 著作権表示やライセンス文言の保持・記載義務すらありません。
- ソースコードの改変、流用、再配布、自社製品への組み込み等、制限なくご活用いただけます。

<p align="right"><small><a name="note" class="muted-link">※一部AIによる生成・調整コードが含まれることがあります。</a></small></p>
