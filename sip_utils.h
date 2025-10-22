/**
 * @file sip_utils.h
 * @brief Header for SIP utility structures.
 */

#ifndef SIP_UTILS_H
#define SIP_UTILS_H

#include <stddef.h>
#include "sip_message.h"

#define MAX_DIALOGS_PER_CALL 16
#define MAX_TXNS_PER_DIALOG 32

typedef enum
{
    SIP_TRANSACTION_STATE_NONE = 0,
    SIP_TRANSACTION_STATE_PROCEEDING,
    SIP_TRANSACTION_STATE_COMPLETED,
    SIP_TRANSACTION_STATE_CONFIRMED,
    SIP_TRANSACTION_STATE_TERMINATED
} sip_transaction_state_t;

typedef enum
{
    SIP_DIALOG_STATE_NONE = 0,
    SIP_DIALOG_STATE_EARLY,
    SIP_DIALOG_STATE_CONFIRMED,
    SIP_DIALOG_STATE_TERMINATED
} sip_dialog_state_t;

typedef enum
{
    SIP_CALL_STATE_IDLE = 0,
    SIP_CALL_STATE_INCOMING,
    SIP_CALL_STATE_RINGING,
    SIP_CALL_STATE_ESTABLISHED,
    SIP_CALL_STATE_TERMINATING,
    SIP_CALL_STATE_TERMINATED
} sip_call_state_t;

typedef struct sip_call_s sip_call_t;
typedef struct sip_dialog_s sip_dialog_t;
typedef struct sip_transaction_s sip_transaction_t;

struct sip_transaction_s
{
    struct sip_transaction_s *next;
    //
    sip_dialog_t *dialog;
    sip_transaction_state_t state;
    sip_message_t *message;
    char branch[SIP_BRANCH_MAX_LENGTH + 1];
    size_t branch_length;
};

struct sip_dialog_s
{
    struct sip_dialog_s *next;
    sip_transaction_t *transaction[MAX_TXNS_PER_DIALOG];
    //
    sip_call_t *call;
    sip_dialog_state_t state;
    char from_tag[SIP_TAG_MAX_LENGTH + 1];
    size_t from_tag_length;
    char to_tag[SIP_TAG_MAX_LENGTH + 1];
    size_t to_tag_length;
};

struct sip_call_s
{
    struct sip_call_s *next;
    sip_dialog_t *dialog[MAX_DIALOGS_PER_CALL];
    //
    sip_call_state_t state;
    char call_id[SIP_CALL_ID_MAX_LENGTH + 1];
    size_t call_id_length;
};

sip_call_t *find_call_by_id(sip_call_t *calls, const char *call_id, size_t call_id_length);
sip_call_t *create_new_call(sip_call_t **calls, const char *call_id, size_t call_id_length);
void delete_call_by_id(sip_call_t **calls, const char *call_id, size_t call_id_length);
void delete_call_by_pointer(sip_call_t **calls, sip_call_t *call);
void cleanup_call(sip_call_t *call);
void delete_all_calls(sip_call_t **calls);
void add_dialog_to_call(sip_call_t *call, sip_dialog_t *dialog);
void remove_dialog_from_call(sip_call_t *call, sip_dialog_t *dialog);

sip_dialog_t *find_dialog_by_id(sip_dialog_t *dialogs, const char *from_tag, size_t from_tag_length, const char *to_tag, size_t to_tag_length);
sip_dialog_t *create_new_dialog(sip_dialog_t **dialogs, const char *from_tag, size_t from_tag_length);
void delete_dialog_by_id(sip_dialog_t **dialogs, const char *from_tag, size_t from_tag_length, const char *to_tag, size_t to_tag_length);
void delete_dialog_by_pointer(sip_dialog_t **dialogs, sip_dialog_t *dialog);
void cleanup_dialog(sip_dialog_t *dialog);
void delete_all_dialogs(sip_dialog_t **dialogs);
void create_to_tag(char *to_tag_buffer, size_t buffer_size);
void add_transaction_to_dialog(sip_dialog_t *dialog, sip_transaction_t *transaction);
void remove_transaction_from_dialog(sip_dialog_t *dialog, sip_transaction_t *transaction);

sip_transaction_t *find_transaction_by_id(sip_transaction_t *transactions, const char *branch, size_t branch_length);
sip_transaction_t *create_new_transaction(sip_transaction_t **transactions, const char *branch, size_t branch_length);
void delete_transaction_by_id(sip_transaction_t **transactions, const char *branch, size_t branch_length);
void delete_transaction_by_pointer(sip_transaction_t **transactions, sip_transaction_t *transaction);
void cleanup_transaction(sip_transaction_t *transaction);
void delete_all_transactions(sip_transaction_t **transactions);
void set_transaction_dialog(sip_transaction_t *transaction, sip_dialog_t *dialog);

#endif // SIP_UTILS_H