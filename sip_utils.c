#include "sip_utils.h"
#include <stdlib.h>
#include <string.h>

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
    new_call->call_id = (char *)malloc(call_id_length + 1);
    if (new_call->call_id == NULL)
    {
        free(new_call);
        return NULL;
    }
    strncpy(new_call->call_id, call_id, call_id_length);
    new_call->call_id[call_id_length] = '\0';
    new_call->call_id_length = call_id_length;
    new_call->dialog = NULL;
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
    free(call->call_id);
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

sip_dialog_t *find_dialog_by_id(sip_dialog_t *dialogs, const char *local_tag, size_t local_tag_length, const char *remote_tag, size_t remote_tag_length)
{
    sip_dialog_t *current = dialogs;
    while (current != NULL)
    {
        if (current->local_tag_length == local_tag_length &&
            strncmp(current->local_tag, local_tag, local_tag_length) == 0)
        {
            if ((current->remote_tag == NULL && remote_tag == NULL) || (current->remote_tag_length == remote_tag_length &&
                                                                        strncmp(current->remote_tag, remote_tag, remote_tag_length) == 0))
            {
                return current;
            }
        }
        current = current->next;
    }
    return NULL;
}

sip_dialog_t *create_new_dialog(sip_dialog_t **dialogs, const char *local_tag, size_t local_tag_length)
{
    sip_dialog_t *new_dialog = (sip_dialog_t *)malloc(sizeof(sip_dialog_t));
    if (new_dialog == NULL)
    {
        return NULL;
    }
    new_dialog->local_tag = (char *)malloc(local_tag_length + 1);
    if (new_dialog->local_tag == NULL)
    {
        free(new_dialog);
        return NULL;
    }
    strncpy(new_dialog->local_tag, local_tag, local_tag_length);
    new_dialog->local_tag[local_tag_length] = '\0';
    new_dialog->local_tag_length = local_tag_length;
    new_dialog->remote_tag = NULL;
    new_dialog->remote_tag_length = 0;
    new_dialog->transaction = NULL;
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
    free(dialog->local_tag);
    if (dialog->remote_tag != NULL)
    {
        free(dialog->remote_tag);
    }
    free(dialog);
}

void delete_dialog(sip_dialog_t *dialogs, const char *local_tag, size_t local_tag_length, const char *remote_tag, size_t remote_tag_length)
{
    sip_dialog_t *current = dialogs;
    sip_dialog_t *previous = NULL;

    while (current != NULL)
    {
        if (current->local_tag_length == local_tag_length &&
            strncmp(current->local_tag, local_tag, local_tag_length) == 0)
        {
            if ((current->remote_tag == NULL && remote_tag == NULL) || (current->remote_tag_length == remote_tag_length &&
                                                                        strncmp(current->remote_tag, remote_tag, remote_tag_length) == 0))
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
    new_transaction->branch = (char *)malloc(branch_length + 1);
    if (new_transaction->branch == NULL)
    {
        free(new_transaction);
        return NULL;
    }
    strncpy(new_transaction->branch, branch, branch_length);
    new_transaction->branch[branch_length] = '\0';
    new_transaction->branch_length = branch_length;
    new_transaction->next = *transactions;
    *transactions = new_transaction;
    return new_transaction;
}

void cleanup_transaction(sip_transaction_t *transaction)
{
    free(transaction->branch);
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