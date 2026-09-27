/**
 * @file xmodem.h
 * @brief ベアメタル環境向け 最小限XMODEM送受信ライブラリ
 * @details 
 * ハードウェア依存（UART送受信）を排除し、任意の組込み環境で共通利用できる、
 * 128バイトブロックの単純チェックサム方式XMODEM送受信実装です。
 * 
 * @copyright Copyright (c) 2026 Embedead-Inside
 * @license SPDX-License-Identifier: MIT-0
 */

#ifndef XMODEM_H
#define XMODEM_H

#include <stdint.h>

/** @name XMODEM 共通定義 */
/// @{
/* 結果・終了 */
#define XMODEM_COMPLETE    (0)   /**< 正常終了（送受信完了） */
#define XMODEM_ERROR       (1)   /**< エラー終了（タイムアウト、チェックサム不一致等） */
#define XMODEM_CANCEL      (2)   /**< キャンセル終了（CAN受信、またはコールバック中断） */

/* イベント（送受信共通） */
#define XMODEM_START       (3)   /**< セッション開始 */
#define XMODEM_PACKET      (4)   /**< パケット処理（受信：データ書込 / 送信：データ供給） */
中 */
/// @}

/** @name プラットフォーム依存I/O関数（ユーザー側で実装が必要） */
/// @{

/**
 * @brief 1バイトのUART送信関数
 * @details 
 * ハードウェアのUARTを使用して、指定された1バイトのデータを送信します。
 * 送信が完了するまでブロッキング（ポーリング）することを想定しています。
 * 
 * @param[in] b 送信データ（1バイト）
 */
void uart_send(uint8_t b);

/**
 * @brief 1バイトのUART受信関数（タイムアウト付き）
 * @details 
 * ハードウェアのUARTを使用して、指定されたタイムアウト時間内に1バイトのデータを受信します。
 * 
 * @param[out] b 受信データを格納する変数のポインタ
 * @param[in]  timeout_ms タイムアウト時間（ミリ秒）
 * @return int 
 *  - 1: 正常に1バイト受信した
 *  - 0: タイムアウト、または受信エラー
 */
int uart_recv(uint8_t *b, uint32_t timeout_ms);

/// @}

/**
 * @brief 状態変化・データ送受信コールバック（固定名称・共通化）
 * @details 
 * 受信側および送信側から共通で呼び出されます。eventの値および呼び出し元文脈により処理を分岐します。
 * 
 * @param[in] event イベント種別 (XMODEM_START または XMODEM_PACKET)
 * @param[in] seq   パケットシーケンス番号 (XMODEM_PACKET時のみ有効)
 * @param[in,out] data 
 *                  - 受信時: 受信した128バイトデータ（読み取り専用）
 *                  - 送信時: 次に送信する128バイトデータを格納するバッファポインタ（書き込み先）
 * @return 0        処理継続
 * @return 非0      処理中断（通信をキャンセルし、XMODEM_CANCELを返す）
 */
int xmodem_callback(uint8_t event, uint8_t seq, uint8_t *data);

/**
 * @brief XMODEM受信処理のメイン関数
 * @details 
 * 受信側として動作し、送信側からのデータ転送を待受けて順次コールバックに通知します。
 * 
 * @return int 終了結果（XMODEM_COMPLETE, XMODEM_ERROR, XMODEM_CANCEL）
 */
int xmodem_receive(void);

/**
 * @brief XMODEM送信処理のメイン関数
 * @details 
 * 送信側として動作し、受信側からの要求（NAK/'C'）を待ってデータを順次送信します。
 * 
 * @return int 終了結果（XMODEM_COMPLETE, XMODEM_ERROR, XMODEM_CANCEL）
 */
int xmodem_transmit(void);

#endif /* XMODEM_H */
