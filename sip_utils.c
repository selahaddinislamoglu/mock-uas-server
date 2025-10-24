#include "sip_utils.h"
#include "log.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

const char *call_states[] = {SIP_CALL_STATE_IDLE_TEXT, SIP_CALL_STATE_INCOMING_TEXT, SIP_CALL_STATE_RINGING_TEXT, SIP_CALL_STATE_ESTABLISHED_TEXT, SIP_CALL_STATE_FAILED_TEXT, SIP_CALL_STATE_TERMINATING_TEXT, SIP_CALL_STATE_TERMINATED_TEXT};
const char *dialog_states[] = {SIP_DIALOG_STATE_IDLE_TEXT, SIP_DIALOG_STATE_EARLY_TEXT, SIP_DIALOG_STATE_CONFIRMED_TEXT, SIP_DIALOG_STATE_TERMINATED_TEXT};
const char *transaction_states[] = {SIP_TRANSACTION_STATE_IDLE_TEXT, SIP_TRANSACTION_STATE_PROCEEDING_TEXT, SIP_TRANSACTION_STATE_COMPLETED_TEXT, SIP_TRANSACTION_STATE_TERMINATED_TEXT};

/**
 * @brief Finds a call by its ID.
 * @param calls The list of calls to search.
 * @param call_id The ID of the call to find.
 * @param call_id_length The length of the call ID.
 * @return A pointer to the call if found, or NULL if not found.
 */
sip_call_t *find_call_by_id(sip_call_t *calls, const char *call_id, size_t call_id_length)
{
    if (calls == NULL || call_id == NULL || call_id_length == 0)
    {
        error("Invalid parameters");
        return NULL;
    }
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

/**
 * @brief Creates a new call and adds it to the list of calls.
 * @param calls The list of calls to add the new call to.
 * @param call_id The ID of the new call.
 * @param call_id_length The length of the call ID.
 * @return A pointer to the new call.
 */
sip_call_t *create_new_call(sip_call_t **calls, const char *call_id, size_t call_id_length)
{
    if (calls == NULL || call_id == NULL || call_id_length == 0)
    {
        error("Invalid parameters");
        return NULL;
    }
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

/**
 * @brief Frees memory allocated for a call.
 * @param call The call to free.
 */
void cleanup_call(sip_call_t *call)
{
    if (call == NULL)
    {
        log("Call is NULL");
        return;
    }
    free(call);
}

/**
 * @brief Deletes a call from the list of calls.
 * @param calls The list of calls to delete the call from.
 * @param call_id The ID of the call to delete.
 * @param call_id_length The length of the call ID.
 */
void delete_call_by_id(sip_call_t **calls, const char *call_id, size_t call_id_length)
{
    if (calls == NULL || call_id == NULL || call_id_length == 0)
    {
        error("Invalid parameters");
        return;
    }

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

/**
 * @brief Deletes a call from the list of calls.
 * @param calls The list of calls to delete the call from.
 * @param call The call to delete.
 */
void delete_call_by_pointer(sip_call_t **calls, sip_call_t *call)
{
    if (calls == NULL || call == NULL)
    {
        error("Invalid parameters");
        return;
    }

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

/**
 * @brief Deletes all calls from the list of calls.
 * @param calls The list of calls to delete.
 */
void delete_all_calls(sip_call_t **calls)
{
    if (calls == NULL)
    {
        error("Invalid parameters");
        return;
    }
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

/**
 * @brief Adds a dialog to a call.
 * @param call The call to add the dialog to.
 * @param dialog The dialog to add.
 */
void add_dialog_to_call(sip_call_t *call, sip_dialog_t *dialog)
{ // TODO boundary check

    if (call == NULL || dialog == NULL)
    {
        error("Invalid parameters");
        return;
    }
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

/**
 * @brief Removes a dialog from a call.
 * @param call The call to remove the dialog from.
 * @param dialog The dialog to remove.
 */
void remove_dialog_from_call(sip_call_t *call, sip_dialog_t *dialog)
{
    if (call == NULL || dialog == NULL)
    {
        error("Invalid parameters");
        return;
    }
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

/**
 * @brief Sets the state of a call.
 * @param call The call to set the state of.
 * @param state The state to set the call to.
 */
void set_call_state(sip_call_t *call, sip_call_state_t state)
{
    if (call == NULL)
    {
        error("Invalid parameters");
        return;
    }
    log("Setting call state from %s to %s id %.*s", call_states[call->state], call_states[state], (int)call->call_id_length, call->call_id);
    call->state = state;
}

/**
 * @brief Finds a dialog by its ID.
 * @param dialogs The list of dialogs to search.
 * @param from_tag The ID of the dialog to find.
 * @param from_tag_length The length of the dialog ID.
 * @param to_tag The ID of the dialog to find.
 * @param to_tag_length The length of the dialog ID.
 * @return The found dialog, or NULL if not found.
 */
sip_dialog_t *find_dialog_by_id(sip_dialog_t *dialogs, const char *from_tag, size_t from_tag_length, const char *to_tag, size_t to_tag_length)
{
    if (dialogs == NULL || from_tag == NULL || from_tag_length == 0)
    {
        error("Invalid parameters");
        return NULL;
    }
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

/**
 * @brief Creates to tag.
 * @param to_tag_buffer The buffer to create the tag in.
 * @param buffer_size The size of the buffer.
 */
void create_to_tag(char *to_tag_buffer, size_t buffer_size)
{
    if (to_tag_buffer == NULL || buffer_size == 0)
    {
        error("Invalid parameters");
        return;
    }
    char c;
    size_t i;
    for (i = 0; i < buffer_size; i++)
    {
        c = '0' + (rand() % 10);
        to_tag_buffer[i] = c;
    }
}

/**
 * @brief Creates a new dialog and adds it to the list of dialogs.
 * @param dialogs The list of dialogs to add the new dialog to.
 * @param from_tag The ID of the new dialog.
 * @param from_tag_length The length of the dialog ID.
 * @return A pointer to the new dialog.
 */
sip_dialog_t *create_new_dialog(sip_dialog_t **dialogs, const char *from_tag, size_t from_tag_length)
{
    if (dialogs == NULL || from_tag == NULL || from_tag_length == 0)
    {
        error("Invalid parameters");
        return NULL;
    }
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

/**
 * @brief Frees memory allocated for a dialog.
 * @param dialog The dialog to free.
 */
void cleanup_dialog(sip_dialog_t *dialog)
{
    if (dialog == NULL)
    {
        log("Dialog is NULL");
        return;
    }
    if (dialog->call != NULL)
    {
        remove_dialog_from_call(dialog->call, dialog);
    }
    free(dialog);
}

/**
 * @brief Deletes a dialog from the list of dialogs.
 * @param dialogs The list of dialogs to delete the dialog from.
 * @param from_tag The ID of the dialog to delete.
 * @param from_tag_length The length of the dialog ID.
 * @param to_tag The ID of the dialog to delete.
 * @param to_tag_length The length of the dialog ID.
 */
void delete_dialog_by_id(sip_dialog_t **dialogs, const char *from_tag, size_t from_tag_length, const char *to_tag, size_t to_tag_length)
{
    if (dialogs == NULL || from_tag == NULL || from_tag_length == 0)
    {
        error("Invalid parameters");
        return;
    }
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

/**
 * @brief Deletes a dialog from the list of dialogs.
 * @param dialogs The list of dialogs to delete the dialog from.
 * @param dialog The dialog to delete.
 */
void delete_dialog_by_pointer(sip_dialog_t **dialogs, sip_dialog_t *dialog)
{
    if (dialogs == NULL || dialog == NULL)
    {
        error("Invalid parameters");
        return;
    }
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

/**
 * @brief Adds a transaction to a dialog.
 * @param dialog The dialog to add the transaction to.
 * @param transaction The transaction to add.
 */
void add_transaction_to_dialog(sip_dialog_t *dialog, sip_transaction_t *transaction)
{
    if (dialog == NULL || transaction == NULL)
    {
        error("Invalid parameters");
        return;
    }
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

/**
 * @brief Removes a transaction from a dialog.
 * @param dialog The dialog to remove the transaction from.
 * @param transaction The transaction to remove.
 */
void remove_transaction_from_dialog(sip_dialog_t *dialog, sip_transaction_t *transaction)
{
    if (dialog == NULL || transaction == NULL)
    {
        error("Invalid parameters");
        return;
    }
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

/**
 * @brief Sets the call of a dialog.
 * @param dialog The dialog to set the call of.
 * @param call The call to set.
 */
void set_dialog_call(sip_dialog_t *dialog, sip_call_t *call)
{
    if (dialog == NULL || call == NULL)
    {
        error("Invalid parameters");
        return;
    }
    dialog->call = call;
    add_dialog_to_call(call, dialog);
}

/**
 * @brief Sets the state of a dialog.
 * @param dialog The dialog to set the state of.
 * @param state The state to set the dialog to.
 */
void set_dialog_state(sip_dialog_t *dialog, sip_dialog_state_t state)
{
    if (dialog == NULL)
    {
        error("Invalid parameters");
        return;
    }
    log("Setting dialog state from %s to %s id %.*s %.*s", dialog_states[dialog->state], dialog_states[state], (int)dialog->from_tag_length, dialog->from_tag, (int)dialog->to_tag_length, dialog->to_tag);
    dialog->state = state;
}

/**
 * @brief Deletes all dialogs from the list of dialogs.
 * @param dialogs The list of dialogs to delete.
 */
void delete_all_dialogs(sip_dialog_t **dialogs)
{
    if (dialogs == NULL)
    {
        error("Invalid parameters");
        return;
    }
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

/**
 * @brief Finds a transaction by its branch.
 * @param transactions The list of transactions to search.
 * @param branch The branch of the transaction to find.
 * @param branch_length The length of the branch.
 * @return The transaction if found, NULL otherwise.
 */
sip_transaction_t *find_transaction_by_id(sip_transaction_t *transactions, const char *branch, size_t branch_length)
{
    if (transactions == NULL || branch == NULL || branch_length == 0)
    {
        error("Invalid parameters");
        return NULL;
    }
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

/**
 * @brief Creates a new transaction.
 * @param transactions The list of transactions to add the new transaction to.
 * @param branch The branch of the new transaction.
 * @param branch_length The length of the branch.
 * @return The new transaction if successful, NULL otherwise.
 */
sip_transaction_t *create_new_transaction(sip_transaction_t **transactions, const char *branch, size_t branch_length)
{
    if (transactions == NULL || branch == NULL || branch_length == 0)
    {
        error("Invalid parameters");
        return NULL;
    }
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

/**
 * @brief Frees memory allocated for a transaction.
 * @param transaction The transaction to free.
 */
void cleanup_transaction(sip_transaction_t *transaction)
{
    if (transaction == NULL)
    {
        log("Transaction is NULL");
        return;
    }
    if (transaction->dialog != NULL)
    {
        remove_transaction_from_dialog(transaction->dialog, transaction);
    }
    cleanup_sip_message(transaction->message);
    free(transaction);
}

/**
 * @brief Deletes a transaction from the list of transactions.
 * @param transactions The list of transactions to delete the transaction from.
 * @param branch The branch of the transaction to delete.
 * @param branch_length The length of the branch.
 */
void delete_transaction_by_id(sip_transaction_t **transactions, const char *branch, size_t branch_length)
{
    if (transactions == NULL || branch == NULL || branch_length == 0)
    {
        error("Invalid parameters");
        return;
    }
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

/**
 * @brief Deletes a transaction from the list of transactions.
 * @param transactions The list of transactions to delete the transaction from.
 * @param transaction The transaction to delete.
 */
void delete_transaction_by_pointer(sip_transaction_t **transactions, sip_transaction_t *transaction)
{
    if (transactions == NULL || transaction == NULL)
    {
        error("Invalid parameters");
        return;
    }
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

/**
 * @brief Deletes all transactions from the list of transactions.
 * @param transactions The list of transactions to delete.
 */
void delete_all_transactions(sip_transaction_t **transactions)
{
    if (transactions == NULL)
    {
        error("Invalid parameters");
        return;
    }
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

/**
 * @brief Sets the dialog of a transaction.
 * @param transaction The transaction to set the dialog of.
 * @param dialog The dialog to set.
 */
void set_transaction_dialog(sip_transaction_t *transaction, sip_dialog_t *dialog)
{
    if (transaction == NULL || dialog == NULL)
    {
        error("Invalid parameters");
        return;
    }
    transaction->dialog = dialog;
    add_transaction_to_dialog(dialog, transaction);
}

/**
 * @brief Sets the state of a transaction.
 * @param transaction The transaction to set the state of.
 * @param state The state to set the transaction to.
 */
void set_transaction_state(sip_transaction_t *transaction, sip_transaction_state_t state)
{
    if (transaction == NULL)
    {
        error("Invalid parameters");
        return;
    }
    log("Setting transaction state from %s to %s id %.*s", transaction_states[transaction->state], transaction_states[state], (int)transaction->branch_length, transaction->branch);
    transaction->state = state;

    switch (transaction->state)
    {
    case SIP_TRANSACTION_STATE_COMPLETED:
        // start timer for ACK
        break;
    case SIP_TRANSACTION_STATE_TERMINATED:
        // start timer for cleanup
        break;
    default:
        break;
    }
}