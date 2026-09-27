/**
 * @file xmodem.h
 * @brief ベアメタル環境向け 最小限XMODEM送受信ライブラリ
 * @details 
 * ハードウェア依存（UART送受信）を排除し、Xilinx、Nios II、RX651等の
 * 異なるマイコンアーキテクチャで共通利用できる、128バイトブロックの
 * 単純チェックサム方式XMODEM送受信実装です。
 */

#ifndef XMODEM_H
#define XMODEM_H

#include <stdint.h>

/** @name XMODEM 終了理由／ステータス定義 */
/// @{
#define XMODEM_RES_COMPLETE  (0)   /**< 正常終了（送受信完了） */
#define XMODEM_RES_ERROR    (-1)   /**< エラー終了（タイムアウト、チェックサム不一致、連続エラー超過等） */
#define XMODEM_RES_CANCEL   (-2)   /**< キャンセル終了（コールバック側、または相手からのCAN受信による中断） */
/// @}

/** @name XMODEM イベント種別定義（受信時コールバック用） */
/// @{
#define XMODEM_EVT_START     (1)   /**< 受信セッション開始 */
#define XMODEM_EVT_PACKET    (2)   /**< 128バイトのデータパケット正常受信 */
/// @}

/**
 * @brief 受信時の状態変化・データ受信コールバック（固定名称）
 * @details 
 * 受信側（xmodem_receive）実行時に呼び出されます。
 * 
 * @param[in] event イベント種別 (XMODEM_EVT_START または XMODEM_EVT_PACKET)
 * @param[in] seq   パケットシーケンス番号 (XMODEM_EVT_PACKET時のみ有効、1〜255の循環)
 * @param[in] data  受信した128バイトのデータバッファへのポインタ (XMODEM_EVT_PACKET時のみ有効)
 * @return 0        処理継続（次のパケットへ）
 * @return 非0      処理中断（通信をキャンセルし、xmodem_receiveはXMODEM_RES_CANCELを返す）
 */
int xmodem_callback(uint8_t event, uint8_t seq, const uint8_t *data);

/**
 * @brief 送信時のデータ供給コールバック（固定名称）
 * @details 
 * 送信側（xmodem_transmit）実行時に、次のパケット要求があった際に呼び出されます。
 * 
 * @param[in] seq   送信すべきパケットシーケンス番号（1始まり）
 * @param[out] dest 128バイトのデータを格納するバッファポインタ
 * @return 0        正常にデータをセットした（送信継続）
 * @return 非0      これ以上送信するデータがない（EOT送信へ移行）
 */
int xmodem_tx_callback(uint8_t seq, uint8_t *dest);

/**
 * @brief XMODEM受信処理のメイン関数
 * @details 
 * 受信側として動作し、送信側からのデータ転送を待受けて順次コールバックに通知します。
 * 
 * @return int 終了理由（XMODEM_RES_COMPLETE, XMODEM_RES_ERROR, XMODEM_RES_CANCEL）
 */
int xmodem_receive(void);

/**
 * @brief XMODEM送信処理のメイン関数
 * @details 
 * 送信側として動作し、受信側からの要求（NAK/'C'）を待ってデータを順次送信します。
 * 
 * @return int 終了理由（XMODEM_RES_COMPLETE, XMODEM_RES_ERROR, XMODEM_RES_CANCEL）
 */
int xmodem_transmit(void);

#endif /* XMODEM_H */
