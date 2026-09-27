/**
 * @file xmodem.c
 * @brief XMODEM送受信モジュールの本体実装
 * 
 * @copyright Copyright (c) 2026 Embedead-Inside
 * @license SPDX-License-Identifier: MIT-0
 */

#include "xmodem.h"

#define SOH        0x01
#define EOT        0x04
#define ACK        0x06
#define NAK        0x15
#define CAN        0x18
#define MAX_ERRORS 10

/**
 * @brief XMODEM 受信メイン処理
 */
int xmodem_receive(void)
{
    uint8_t seq_exp = 1, errs = 0, h, seq, inv, sum, rcsum, buf[128];
    int i;

    xmodem_callback(XMODEM_START, 0, (void*)0);
    uart_send(NAK);

    while (1) {
        if (!uart_recv(&h, 3000)) {
            if (++errs > MAX_ERRORS) return XMODEM_ERROR;
            uart_send(NAK);
            continue;
        }

        if (h == EOT) {
            uart_send(ACK);
            return XMODEM_COMPLETE;
        }

        if (h != SOH) {
            uart_send(NAK);
            continue;
        }

        if (!uart_recv(&seq, 1000) || !uart_recv(&inv, 1000) || (seq + inv != 0xFF)) {
            uart_send(NAK);
            continue;
        }

        sum = 0;
        for (i = 0; i < 128; i++) {
            if (!uart_recv(&buf[i], 1000)) return XMODEM_ERROR;
            sum += buf[i];
        }

        if (!uart_recv(&rcsum, 1000) || sum != rcsum || seq != seq_exp) {
            uart_send(NAK);
            if (++errs > MAX_ERRORS) return XMODEM_ERROR;
            continue;
        }

        if (seq == (uint8_t)(seq_exp - 1)) {
            uart_send(ACK);
            continue;
        }

        if (xmodem_callback(XMODEM_PACKET, seq, buf) != 0) {
            uart_send(CAN);
            return XMODEM_CANCEL;
        }

        seq_exp++;
        errs = 0;
            uart_send(ACK);
    }
}

/**
 * @brief XMODEM 送信メイン処理
 */
int xmodem_transmit(void)
{
    uint8_t seq = 1, errs = 0, resp, sum, buf[128];
    int i;

    xmodem_callback(XMODEM_START, 0, (void*)0);

    /* 接続待受（NAK または 'C' を待つ） */
    while (1) {
        if (!uart_recv(&resp, 3000)) {
            if (++errs > MAX_ERRORS) return XMODEM_ERROR;
            continue;
        }
        if (resp == NAK || resp == 'C') {
            break;
        }
        if (resp == CAN) {
            return XMODEM_ERROR;
        }
    }
    errs = 0;

    while (1) {
        /* コールバック経由で次データ取得（非0返却時はデータ終了とみなしEOTへ） */
        if (xmodem_callback(XMODEM_PACKET, seq, buf) != 0) {
            uart_send(EOT);
            
            if (uart_recv(&resp, 3000) && resp == ACK) {
                return XMODEM_COMPLETE;
            } else {
                return XMODEM_ERROR;
            }
        }

        /* パケット送信 (SOH + Seq + ~Seq + Data[128] + Checksum) */
        uart_send(SOH);
        uart_send(seq);
        uart_send((uint8_t)~seq);

        for (i = sum = 0; i < 128; i++) {
            uart_send(buf[i]);
            sum += buf[i];
        }
        uart_send(sum);

        /* 応答受信 */
        if (!uart_recv(&resp, 3000)) {
            if (++errs > MAX_ERRORS) return XMODEM_ERROR;
            continue;
        }

        if (resp == ACK) {
            seq++;
            errs = 0;
        } else if (resp == CAN) {
            return XMODEM_CANCEL;
        } else {
            if (++errs > MAX_ERRORS) return XMODEM_ERROR;
        }
    }
}
