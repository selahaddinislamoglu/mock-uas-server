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
    if (call->dialog != NULL)
    {
        delete_all_dialogs(&call->dialog);
    }
    free(call);
}

void delete_call(sip_call_t **calls, const char *call_id, size_t call_id_length)
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
    new_dialog->next = *dialogs;
    *dialogs = new_dialog;
    return new_dialog;
}

void cleanup_dialog(sip_dialog_t *dialog)
{
    if (dialog->transaction != NULL)
    {
        delete_all_transactions(&dialog->transaction);
    }
    free(dialog);
}

void delete_dialog(sip_dialog_t *dialogs, const char *from_tag, size_t from_tag_length, const char *to_tag, size_t to_tag_length)
{
    sip_dialog_t *current = dialogs;
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
                    dialogs = current->next;
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

sip_transaction_t *find_transaction_by_branch(sip_transaction_t *transactions, const char *branch, size_t branch_length)
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
    cleanup_sip_message(transaction->last_message);
    free(transaction);
}

void delete_transaction(sip_transaction_t **transactions, const char *branch, size_t branch_length)
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