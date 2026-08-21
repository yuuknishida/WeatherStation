/*
 * mailbox.h
 *
 *  Created on: Mar 19, 2026
 *      Author: ynish
 */

#ifndef APP_MAILBOX_H_
#define APP_MAILBOX_H_

#include "OS.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct mailbox{
    bool full;
    task_t *waitTask;
    uint16_t data;
} Mailbox_t;

extern void Mailbox_Create(Mailbox_t  *mailbox);
extern void Mailbox_Send(Mailbox_t *mailbox, uint32_t data);
extern uint32_t Mailbox_Receive(Mailbox_t *mailbox);
extern bool Mailbox_TryReceive(Mailbox_t *mailbox, uint32_t *out);

#endif /* APP_MAILBOX_H_ */
