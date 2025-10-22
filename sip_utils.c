#include "sip_utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

sip_call_t *find_call_by_id(sip_call_t *calls, const char *call_id, size_t call_id_length)
{
    sip_call_t *current = calls;
    while (current != NULL)
    {
        if (current->call_id_length == call_id_length &&
            strncmp(current->call_id, call_id, call_id_length) == 0)
        {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

sip_call_t *create_new_call(sip_call_t **calls, const char *call_id, size_t call_id_length)
{
    sip_call_t *new_call = (sip_call_t *)malloc(sizeof(sip_call_t));
    if (new_call == NULL)
    {
        return NULL;
    }
    memset(new_call, 0, sizeof(sip_call_t));
    snprintf(new_call->call_id, sizeof(new_call->call_id), "%.*s", (int)call_id_length, call_id);
    new_call->call_id_length = call_id_length;
    new_call->next = *calls;
    *calls = new_call;
    return new_call;
}

void cleanup_call(sip_call_t *call)
{
    free(call);
}

void delete_call_by_id(sip_call_t **calls, const char *call_id, size_t call_id_length)
{
    sip_call_t *current = *calls;
    sip_call_t *previous = NULL;

    while (current != NULL)
    {
        if (current->call_id_length == call_id_length &&
            strncmp(current->call_id, call_id, call_id_length) == 0)
        {
            if (previous == NULL)
            {
                *calls = current->next;
            }
            else
            {
                previous->next = current->next;
            }
            cleanup_call(current);
            return;
        }
        previous = current;
        current = current->next;
    }
}

void delete_call_by_pointer(sip_call_t **calls, sip_call_t *call)
{
    sip_call_t *current = *calls;
    sip_call_t *previous = NULL;

    while (current != NULL)
    {
        if (current == call)
        {
            if (previous == NULL)
            {
                *calls = current->next;
            }
            else
            {
                previous->next = current->next;
            }
            cleanup_call(current);
            return;
        }
        previous = current;
        current = current->next;
    }
}

void delete_all_calls(sip_call_t **calls)
{
    sip_call_t *current = *calls;
    sip_call_t *next;

    while (current != NULL)
    {
        next = current->next;
        cleanup_call(current);
        current = next;
    }
    *calls = NULL;
}

void add_dialog_to_call(sip_call_t *call, sip_dialog_t *dialog)
{ // TODO boundary check
    int reserved = -1;
    for (size_t i = 0; i < MAX_DIALOGS_PER_CALL; i++)
    {
        if (reserved == -1 && call->dialog[i] == NULL)
        {
            reserved = i;
        }

        if (call->dialog[i] == dialog)
        {
            return;
        }
    }
    if (reserved != -1)
    {
        call->dialog[reserved] = dialog;
    }
}

void remove_dialog_from_call(sip_call_t *call, sip_dialog_t *dialog)
{
    // TODO boundary check
    for (size_t i = 0; i < MAX_DIALOGS_PER_CALL; i++)
    {
        if (call->dialog[i] == dialog)
        {
            call->dialog[i] = NULL;
            return;
        }
    }
}

sip_dialog_t *find_dialog_by_id(sip_dialog_t *dialogs, const char *from_tag, size_t from_tag_length, const char *to_tag, size_t to_tag_length)
{
    sip_dialog_t *current = dialogs;
    while (current != NULL)
    {
        if (current->from_tag_length == from_tag_length &&
            strncmp(current->from_tag, from_tag, from_tag_length) == 0)
        {
            if ((current->to_tag == NULL && to_tag == NULL) || (current->to_tag_length == to_tag_length &&
                                                                strncmp(current->to_tag, to_tag, to_tag_length) == 0))
            {
                return current;
            }
        }
        current = current->next;
    }
    return NULL;
}

void create_to_tag(char *to_tag_buffer, size_t buffer_size)
{
    char c;
    size_t i;
    for (i = 0; i < buffer_size; i++)
    {
        c = '0' + (rand() % 10);
        to_tag_buffer[i] = c;
    }
}

sip_dialog_t *create_new_dialog(sip_dialog_t **dialogs, const char *from_tag, size_t from_tag_length)
{
    sip_dialog_t *new_dialog = (sip_dialog_t *)malloc(sizeof(sip_dialog_t));
    if (new_dialog == NULL)
    {
        return NULL;
    }
    memset(new_dialog, 0, sizeof(sip_dialog_t));
    snprintf(new_dialog->from_tag, sizeof(new_dialog->from_tag), "%.*s", (int)from_tag_length, from_tag);
    new_dialog->from_tag_length = from_tag_length;
    create_to_tag(new_dialog->to_tag, SIP_BUILD_TAG_LENGTH);
    new_dialog->to_tag_length = SIP_BUILD_TAG_LENGTH;
    new_dialog->next = *dialogs;
    *dialogs = new_dialog;
    return new_dialog;
}

void cleanup_dialog(sip_dialog_t *dialog)
{
    if (dialog->call != NULL)
    {
        remove_dialog_from_call(dialog->call, dialog);
    }
    free(dialog);
}

void delete_dialog_by_id(sip_dialog_t **dialogs, const char *from_tag, size_t from_tag_length, const char *to_tag, size_t to_tag_length)
{
    sip_dialog_t *current = *dialogs;
    sip_dialog_t *previous = NULL;

    while (current != NULL)
    {
        if (current->from_tag_length == from_tag_length &&
            strncmp(current->from_tag, from_tag, from_tag_length) == 0)
        {
            if ((current->to_tag == NULL && to_tag == NULL) || (current->to_tag_length == to_tag_length &&
                                                                strncmp(current->to_tag, to_tag, to_tag_length) == 0))
            {
                if (previous == NULL)
                {
                    *dialogs = current->next;
                }
                else
                {
                    previous->next = current->next;
                }
                cleanup_dialog(current);
                return;
            }
        }
        previous = current;
        current = current->next;
    }
}

void delete_dialog_by_pointer(sip_dialog_t **dialogs, sip_dialog_t *dialog)
{
    sip_dialog_t *current = *dialogs;
    sip_dialog_t *previous = NULL;

    while (current != NULL)
    {
        if (current == dialog)
        {
            if (previous == NULL)
            {
                *dialogs = current->next;
            }
            else
            {
                previous->next = current->next;
            }
            cleanup_dialog(current);
            return;
        }
        previous = current;
        current = current->next;
    }
}

void add_transaction_to_dialog(sip_dialog_t *dialog, sip_transaction_t *transaction)
{
    // TODO boundary check
    int reserved = -1;
    for (size_t i = 0; i < MAX_TXNS_PER_DIALOG; i++)
    {
        if (reserved == -1 && dialog->transaction[i] == NULL)
        {
            reserved = i;
        }

        if (dialog->transaction[i] == transaction)
        {
            return;
        }
    }
    if (reserved != -1)
    {
        dialog->transaction[reserved] = transaction;
    }
}

void remove_transaction_from_dialog(sip_dialog_t *dialog, sip_transaction_t *transaction)
{
    // TODO boundary check
    for (size_t i = 0; i < MAX_TXNS_PER_DIALOG; i++)
    {
        if (dialog->transaction[i] == transaction)
        {
            dialog->transaction[i] = NULL;
            return;
        }
    }
}

void set_dialog_call(sip_dialog_t *dialog, sip_call_t *call)
{
    dialog->call = call;
    add_dialog_to_call(call, dialog);
}

void delete_all_dialogs(sip_dialog_t **dialogs)
{
    sip_dialog_t *current = *dialogs;
    sip_dialog_t *next;

    while (current != NULL)
    {
        next = current->next;
        cleanup_dialog(current);
        current = next;
    }
    *dialogs = NULL;
}

sip_transaction_t *find_transaction_by_id(sip_transaction_t *transactions, const char *branch, size_t branch_length)
{
    sip_transaction_t *current = transactions;
    while (current != NULL)
    {
        if (current->branch_length == branch_length &&
            strncmp(current->branch, branch, branch_length) == 0)
        {
            return current;
        }
        current = current->next;
    }
    return NULL;
}
sip_transaction_t *create_new_transaction(sip_transaction_t **transactions, const char *branch, size_t branch_length)
{
    sip_transaction_t *new_transaction = (sip_transaction_t *)malloc(sizeof(sip_transaction_t));
    if (new_transaction == NULL)
    {
        return NULL;
    }
    memset(new_transaction, 0, sizeof(sip_transaction_t));
    snprintf(new_transaction->branch, sizeof(new_transaction->branch), "%.*s", (int)branch_length, branch);
    new_transaction->branch_length = branch_length;
    new_transaction->next = *transactions;
    *transactions = new_transaction;
    return new_transaction;
}

void cleanup_transaction(sip_transaction_t *transaction)
{
    if (transaction->dialog != NULL)
    {
        remove_transaction_from_dialog(transaction->dialog, transaction);
    }
    cleanup_sip_message(transaction->message);
    free(transaction);
}

void delete_transaction_by_id(sip_transaction_t **transactions, const char *branch, size_t branch_length)
{
    sip_transaction_t *current = *transactions;
    sip_transaction_t *previous = NULL;

    while (current != NULL)
    {
        if (current->branch_length == branch_length &&
            strncmp(current->branch, branch, branch_length) == 0)
        {
            if (previous == NULL)
            {
                *transactions = current->next;
            }
            else
            {
                previous->next = current->next;
            }
            cleanup_transaction(current);
            return;
        }
        previous = current;
        current = current->next;
    }
}

void delete_transaction_by_pointer(sip_transaction_t **transactions, sip_transaction_t *transaction)
{
    sip_transaction_t *current = *transactions;
    sip_transaction_t *previous = NULL;

    while (current != NULL)
    {
        if (current == transaction)
        {
            if (previous == NULL)
            {
                *transactions = current->next;
            }
            else
            {
                previous->next = current->next;
            }
            cleanup_transaction(current);
            return;
        }
        previous = current;
        current = current->next;
    }
}

void delete_all_transactions(sip_transaction_t **transactions)
{
    sip_transaction_t *current = *transactions;
    sip_transaction_t *next;

    while (current != NULL)
    {
        next = current->next;
        cleanup_transaction(current);
        current = next;
    }
    *transactions = NULL;
}

void set_transaction_dialog(sip_transaction_t *transaction, sip_dialog_t *dialog)
{
    transaction->dialog = dialog;
    add_transaction_to_dialog(dialog, transaction);
}