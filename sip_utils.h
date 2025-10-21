/**
 * @file sip_utils.h
 * @brief Header for SIP utility structures.
 */

#ifndef SIP_UTILS_H
#define SIP_UTILS_H

#include <stddef.h>
#include "sip_message.h"

struct sip_transaction
{
    struct sip_transaction *next;
    //
    sip_message_t *last_message;
    char *branch;
    size_t branch_length;
};
typedef struct sip_transaction sip_transaction_t;

struct sip_dialog
{
    struct sip_dialog *next;
    sip_transaction_t *transaction;
    //
    char *from_tag;
    size_t from_tag_length;
    char *to_tag;
    size_t to_tag_length;
};
typedef struct sip_dialog sip_dialog_t;

struct sip_call
{
    struct sip_call *next;
    sip_dialog_t *dialog;
    //
    char *call_id;
    size_t call_id_length;
};
typedef struct sip_call sip_call_t;

sip_call_t *find_call_by_id(sip_call_t *calls, const char *call_id, size_t call_id_length);
sip_call_t *create_new_call(sip_call_t **calls, const char *call_id, size_t call_id_length);
void delete_call(sip_call_t **calls, const char *call_id, size_t call_id_length);
void cleanup_call(sip_call_t *call);
void delete_all_calls(sip_call_t **calls);

sip_dialog_t *find_dialog_by_id(sip_dialog_t *dialogs, const char *from_tag, size_t from_tag_length, const char *to_tag, size_t to_tag_length);
sip_dialog_t *create_new_dialog(sip_dialog_t **dialogs, const char *from_tag, size_t from_tag_length);
void delete_dialog(sip_dialog_t *dialogs, const char *from_tag, size_t from_tag_length, const char *to_tag, size_t to_tag_length);
void cleanup_dialog(sip_dialog_t *dialog);
void delete_all_dialogs(sip_dialog_t **dialogs);

sip_transaction_t *find_transaction_by_branch(sip_transaction_t *transactions, const char *branch, size_t branch_length);
sip_transaction_t *create_new_transaction(sip_transaction_t **transactions, const char *branch, size_t branch_length);
void delete_transaction(sip_transaction_t **transactions, const char *branch, size_t branch_length);
void cleanup_transaction(sip_transaction_t *transaction);
void delete_all_transactions(sip_transaction_t **transactions);

#endif // SIP_UTILS_H