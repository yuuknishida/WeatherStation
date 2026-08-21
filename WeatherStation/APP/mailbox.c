/*
 * mailbox.c
 *
 *  Created on: Mar 19, 2026
 *      Author: ynish
 */

#include "mailbox.h"
#include "OS.h"
#include <stdlib.h>
#include "driverlib/interrupt.h"

/* Initialize mailbox before use */
void Mailbox_Create(Mailbox_t *mailbox)
{
    mailbox->full = false;
    mailbox->data = 0;
    mailbox->waitTask = NULL;
}

/*
 * Mailbox_Send - non-blocking write
 * Overwrites any unread value*/
void Mailbox_Send(Mailbox_t *mailbox, uint32_t data)
{
    IntMasterDisable();

    mailbox->data = data;
    mailbox->full = true;

    if (mailbox->waitTask != NULL) {
        mailbox->waitTask->state = READY;
        mailbox->waitTask = NULL;
    }

    IntMasterEnable();
}

/*
 * Mailbox_Receive - blocking read
 * if mailbox is empty the calling task blocks and yield
 * returns received value */
uint32_t Mailbox_Receive(Mailbox_t *mailbox)
{
    IntMasterDisable();

    while(!mailbox->full) {
        mailbox->waitTask = currentTask;
        currentTask->state = BLOCKED;

        IntMasterEnable();
        OS_Yield();
        IntMasterDisable();
    }

    uint32_t value = mailbox->data;
    mailbox->full = false;

    IntMasterEnable();
    return value;
}

/*
 * Mailbox_TryReceive
 * Returns true and writes *out if data is available
 * Returns false immediately if the mailbox is empty
*/
bool Mailbox_TryReceive(Mailbox_t *mailbox, uint32_t *out)
{
    IntMasterDisable();

    if (!mailbox->full) {
        IntMasterEnable();
        return false;
    }
    *out = mailbox->data;
    mailbox->full = false;

    IntMasterEnable();
    return true;
}
