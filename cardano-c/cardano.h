/*! \file cardano.h
*/
#ifndef CARDANO_RUST_H
#define CARDANO_RUST_H
/* Basic Types */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

/*!
* Type used to represent failure and success
*/
typedef enum _cardano_result {
    CARDANO_RESULT_SUCCESS = 0,
    CARDANO_RESULT_ERROR = 1
} cardano_result;

/*********/
/* BIP39 */
/*********/

/* bip39 error definitions */
typedef enum _bip39_config_error
{
    BIP39_SUCCESS = 0,
    BIP39_INVALID_MNEMONIC = 1,
    BIP39_INVALID_CHECKSUM = 2,
    BIP39_INVALID_WORD_COUNT = 3
} cardano_bip39_error_t;

typedef uint8_t* cardano_entropy;

/*!
* \brief get entropy array from the given english mnemonics 
* \param [in] mnemonics a string consisting of 9, 12, 15, 18, 21 or 24 english words
* \param [out] entropy the returned entropy array, use `cardano_delete_entropy_array` to release the memory
* \param [out] entropy_size the size of the the returned array
* \sa cardano_delete_entropy_array()
* \returns BIP39_SUCCESS or either BIP39_INVALID_MNEMONIC or BIP39_INVALID_CHECKSUM 
*/
cardano_bip39_error_t cardano_entropy_from_english_mnemonics(
    const char *mnemonics,
    cardano_entropy *entropy,
    uint32_t *entropy_size
);

/*!
* \brief encode a entropy into its equivalent words represented by their index (0 to 2047) in the BIP39 dictionary
* \param [in] number_of_words one of 9, 12, 15, 18, 21 or 24 representing the number of words of the equivalent mnemonic
* \param [in] random_generator a function that generates random bytes  
* \param [out] entropy the returned entropy array
* \param [out] entropy_size the size of the the returned array
* \returns BIP39_SUCCESS or BIP39_INVALID_WORD_COUNT 
*/
cardano_bip39_error_t cardano_entropy_from_random(
    uint8_t number_of_words,
    uint8_t (*random_generator)(),
    cardano_entropy *entropy,
    uint32_t *entropy_size
);

/*!
* delete the allocated memory of entropy byte array
* \param [in] entropy the entropy array
* \param [in] entropy_size the length of the entropy array
* \sa cardano_entropy_from_random()
* \sa cardano_entropy_from_english_mnemonics()
*/
void cardano_delete_entropy_array(uint8_t *entropy, uint32_t entropy_size);

/*!
* \brief encode a entropy into its equivalent words represented by their index (0 to 2047) in the BIP39 dictionary
* \param [in] entropy_raw A pointer to a byte array of either 16, 20, 24, 28 or 32 bytes
* \param [in] entropy_size of the entropy array
* \param [out] mnemonic_index the indexes of the encoded words  
* \param [in] mnemonic_size the number of encoded words 
* \returns success or failure 
*/
cardano_result cardano_bip39_encode(const char * const entropy_raw, unsigned long entropy_size, unsigned short *mnemonic_index, unsigned long mnemonic_size);

/*********/
/* Keys  */
/*********/

/*!
* Size of cardano_xprv
* Extended secret key (64 bytes) followed by a chain code (32 bytes)
* \sa cardano_xprv()
*/
#define XPRV_SIZE 96

/*!
* HDWallet extended private key
*
* Effectively this is ed25519 extended secret key (64 bytes) followed by a chain code (32 bytes)
*
*/
typedef struct cardano_xprv cardano_xprv;

/*!
* Extended Public Key (Point + ChainCode)
*/
typedef struct cardano_xpub cardano_xpub;

/*!
* BIP32 private to private derivation
*/
cardano_xprv *cardano_xprv_derive(cardano_xprv *xprv, uint32_t index); 

/*!
* Free the associated memory
*/
void cardano_xprv_delete(cardano_xprv *privkey);

/*!
* Get the associated cardano_xpub
*/
cardano_xpub *cardano_xprv_to_xpub(cardano_xprv *privkey);

/*!
* Get the bytes representation of cardano_xprv
* \sa cardano_xprv_bytes_delete
*/
uint8_t *cardano_xprv_to_bytes(cardano_xprv *privkey);

/*!
* Free the memory allocated with `cardano_xprv_to_bytes`
*/
void cardano_xprv_bytes_delete(uint8_t  *bytes);

/*!
* \brief Construct cardano_xprv from the given bytes
* \returns 1 if the representation is invalid 0 otherwise
* \sa cardano_xprv_delete
*/
cardano_result cardano_xprv_from_bytes(uint8_t *bytes, cardano_xprv **xprv_out);

/*!
* Free the associated memory
*/
void cardano_xpub_delete(cardano_xpub *pubkey);

/*************/
/* addresses */
/*************/

typedef struct cardano_address cardano_address;

/*! check if an address is a valid protocol address.
 * return 0 on success, !0 on failure. */
int cardano_address_is_valid(const char * address_base58);

cardano_address *cardano_address_new_from_pubkey(cardano_xpub *publickey);
void cardano_address_delete(cardano_address *address);

char *cardano_address_export_base58(cardano_address *address);
cardano_address *cardano_address_import_base58(const char * address_bytes);

/***********/
/* Wallet  */
/***********/

/*!
* HD BIP44 compliant wallet
*/
typedef struct cardano_wallet cardano_wallet;
typedef struct cardano_account cardano_account;

/*!
* Create a wallet with a seed generated from the given entropy and password. 
* The password can be empty and can be used to benefit from plausible deniability
* \param [in] entropy_ptr A pointer to a uint8_t array of either 16, 20, 24, 28 or 32 bytes
* \param [in] entropy_size The former size of the entropy array
* \param [in] password_ptr  A string with the password
* \param [in] password_size The size of the password string
* \param [out] wallet pointer to the created cardano_wallet that must be freed with `cardano_wallet_delete`
* \returns CARDANO_RESULT_SUCCESS | CARDANO_RESULT_ERROR if the entropy is of an invalid size
*/
cardano_result cardano_wallet_new(const uint8_t * const entropy_ptr, unsigned long entropy_size,
                                   const char * const password_ptr, unsigned long password_size,
                                   cardano_wallet** wallet);
/*!
* Free the memory of a wallet allocated with `cardano_wallet_new`
*/
void cardano_wallet_delete(cardano_wallet *);

/*!
* Get the wallet root xprv
* \param [in] wallet_ptr a wallet constructed with `cardano_wallet_new()`
* Call `cardano_xprv_delete` to deallocate the memory 
* \sa cardano_xprv_delete()
*/
cardano_xprv *cardano_wallet_root_key(cardano_wallet *wallet_ptr);

/*!
* \brief Create a new account, the account is given an alias and an index.
*
* The index is the derivation index, we do not check if there is already
* an account with this given index.
* The alias here is only an handy tool, to retrieve a created account from a wallet,
* it's not used for the account derivation process.
*
* \param [in] wallet A pointer to a wallet created with `cardano_wallet_new` 
* \param [in] alias A C string that can be used to retrieve an account from a wallet
* \param [in] index The derivation key 
* \returns pointer to the created account that must be freed with `cardano_account_delete` 
*/
cardano_account *cardano_account_create(cardano_wallet *wallet, const char *alias, unsigned int index);

/*!
* Free the memory allocated with `cardano_account_create`
* \param [in] account a pointer to the account to delete
*/
void cardano_account_delete(cardano_account *account);

/*!
* \brief Generate addressess
* The generated addresses are C strings in base58
* \param [in] account an account created with `cardano_account_create`
* \param [in] internal !0 for external address, 0 for internal 
* \param [in] from_index  
* \param [in] num_indices
* \param [out] addresses_ptr array of strings consisting of the base58 representation of the addresses
* \param [in] protocol_magic
* \returns the number of generated addresses
* \sa cardano_address_import_base58()
* \sa cardano_address_delete() 
*/
unsigned long cardano_account_generate_addresses(cardano_account *account, int internal, unsigned int from_index, unsigned long num_indices, char *addresses_ptr[], uint32_t protocol_magic);
void cardano_account_delete_addresses(char *addresses_ptr[], unsigned long length);

/****************/
/* Transactions */
/****************/

/* transaction error definitions */
typedef enum _transaction_config_error
{
    CARDANO_TRANSACTION_SUCCESS = 0,
    CARDANO_TRANSACTION_NO_OUTPUT = 1,
    CARDANO_TRANSACTION_NO_INPUT = 2,
    /*!The number of signatures doesn't match the number of inputs*/
    CARDANO_TRANSACTION_SIGNATURE_MISMATCH = 3,
    /*!The transaction is too big*/
    CARDANO_TRANSACTION_OVER_LIMIT = 4,
    /*!The number of signatures is greater than the number of inputs*/
    CARDANO_TRANSACTION_SIGNATURES_EXCEEDED = 5,
    /*!The given value is greater than the maximum allowed coin value*/
    CARDANO_TRANSACTION_COIN_OUT_OF_BOUNDS = 6,
} cardano_transaction_error_t;

typedef struct cardano_transaction_builder cardano_transaction_builder;
typedef struct cardano_transaction_finalized cardano_transaction_finalized;
/*!
* Used for addressing a specific output of a transaction built from a TxId (hash of the tx) and the offset in the outputs of this transaction.
* \sa cardano_transaction_output_ptr_new()
*/
typedef struct cardano_txoptr cardano_txoptr;
/*!
* Used for representing a transaction's output
* \sa cardano_transaction_output_new()
*/
typedef struct cardano_txoutput cardano_txoutput;
typedef struct cardano_transaction cardano_transaction;

/*!
* \struct cardano_signed_transaction
*/
typedef struct cardano_signed_transaction cardano_signed_transaction;

//Type for enforcing array size
typedef struct cardano_txid {
    uint8_t bytes[32];
} cardano_txid_t;

/*! \brief Get the transaction id
* \param [in] txaux
* \param [out] out_txid
* \relates cardano_signed_transaction
*/
void cardano_signed_transaction_txid(
    cardano_signed_transaction *txaux,
    cardano_txid_t *out_txid
);

/*! \brief Get the transaction id
* \param [in] tx
* \param [out] out_txid
* \relates cardano_transaction
*/
void cardano_transaction_txid(
    cardano_transaction *tx,
    cardano_txid_t *out_txid
);

/*! \brief Get references to the inputs in the the signed transaction
* \param [in] txaux a cardano signed transaction
* \param [out] out_array array of pointers to cardano_txoptr that you can read with 
* `cardano_transaction_txoptr_txid` and `cardano_transaction_txoptr_index`.
* You should NOT call cardano_transaction_output_ptr_delete with any of this pointers.
* <br> Use `cardano_signed_transaction_delete_inputs` to delete the array of pointers
* \param [out] out_size
* \sa cardano_signed_transaction_delete_inputs()
* \sa cardano_transaction_txoptr_txid()
* \sa cardano_transaction_txoptr_index()
* \sa cardano_signed_transaction_get_outputs()
*/
void cardano_signed_transaction_get_inputs(
    cardano_signed_transaction *txaux,
    cardano_txoptr *(*out_array[]),
    size_t *out_size
);

/*! \brief Release the memory allocated by `cardano_transaction_signed_get_inputs`
*/
void cardano_signed_transaction_delete_inputs(cardano_txoptr *inputs[], size_t size);

/*! \brief Get references to the outputs in the the signed transaction
* \param [in] txaux a cardano signed transaction
* \param [out] out_array array of pointers to cardano_txoutput that you can read with 
`cardano_transaction_txoutput_address` and `cardano_transaction_txoptr_value`.
You should not call cardano_transaction_output_delete with any of this pointers
Use `cardano_transaction_signed_delete_inputs` to delete the array of pointers
* \param [out] out_size
* \sa cardano_signed_transaction_delete_outputs()
* \sa cardano_transaction_txoutput_address()
* \sa cardano_transaction_txoutput_value()
* \sa cardano_signed_transaction_get_inputs()
*/
void cardano_signed_transaction_get_outputs(
    cardano_signed_transaction *txaux,
    cardano_txoutput *(*outputs[]),
    size_t *outputs_size
);

/*! \brief Encode a cardano_signed_transaction as CBOR 
* \param [in] txaux a cardano signed transaction
* \param [out] out_ptr pointer to the serialized data
* \param [out] out_size size of the encoded data
* \sa cardano_signed_transaction_serialized_delete()
*/
cardano_result cardano_signed_transaction_serialize(
    cardano_signed_transaction *txaux,
    uint8_t **out_ptr,
    size_t *out_size);

/*! \brief Release the memory allocated by cardano_signed_transaction_serialize()
* \param [in] pointer
* \param [in] size
*/
void cardano_signed_transaction_serialized_delete(uint8_t *pointer, size_t size);

/*! \brief Release the memory allocated by `cardano_transaction_signed_get_outputs`
*/
void cardano_signed_transaction_delete_outputs(cardano_txoutput *outputs[], size_t size);

/*!
* Create object used for addressing a specific output of a transaction built from a TxId (hash of the tx) and the offset in the outputs of this transaction.
* The memory must be freed with cardano_transaction_output_ptr_delete
* \sa cardano_transaction_output_ptr_delete()
*/
cardano_txoptr * cardano_transaction_output_ptr_new(uint8_t txid[32], uint32_t index);

/*!
* Free the memory allocated with `cardano_transaction_output_ptr_new`
*/
void cardano_transaction_output_ptr_delete(cardano_txoptr *txo);


/*!
* Get the txid of the given cardano_txoptr
* \param [in] txoptr
* \param [out] output
*/
void cardano_transaction_txoptr_txid(cardano_txoptr *txoptr, cardano_txid_t *output);

/*!
* Get the index of the given cardano_txoptr
*/
uint32_t cardano_transaction_txoptr_index(cardano_txoptr *txoptr);

/*!
* Create output for a transaction 
* The memory must be freed with `cardano_transaction_output_delete`
* \sa cardano_transaction_output_delete()
*/
cardano_txoutput * cardano_transaction_output_new(cardano_address *c_addr, uint64_t value);

/*!
* Free the memory allocated with `cardano_transaction_output_delete`
*/
void cardano_transaction_output_delete(cardano_txoutput *output);

/*!
* \brief get a reference to the address of a cardano_txoutput
* You should NOT `cardano_address_delete` on the pointer returned by this function 
* as the data is this function doesn't allocate memory, and the returned pointer is only
* valid as long as the cardano_txoutput is valid
*/
cardano_address *cardano_transaction_txoutput_address(cardano_txoutput *txoutput);

/*!
* \brief get a reference to the address of a cardano_txoutput
*/
uint64_t cardano_transaction_txoutput_value(cardano_txoutput *txoutput);

/*!
* \brief Create builder for a transaction
* \returns builder object
* \sa cardano_transaction_builder_delete()
* \sa cardano_transaction_builder_add_output()
* \sa cardano_transaction_builder_add_input()
* \sa cardano_transaction_builder_add_change_addr()
* \sa cardano_transaction_builder_fee()
* \sa cardano_transaction_builder_finalize()
*/
cardano_transaction_builder * cardano_transaction_builder_new(void);

/*!
* \brief Delete cardano_transaction_builder and free the associated memory
*/
void cardano_transaction_builder_delete(cardano_transaction_builder *tb);

/*!
* \brief Add output to transaction
* \param [in] tb the builder for the transaction
* \param [in] txo created with `cardano_transaction_output_new`
* \sa cardano_transaction_output_new()
*/
void cardano_transaction_builder_add_output(cardano_transaction_builder *tb, cardano_txoutput *txo);

/*!
* \brief Add input to the transaction
* \param [in] tb the builder for the transaction
* \param [in] c_txo created with `cardano_transaction_output_ptr_new`
* \param [in] value the cost 
* \sa cardano_transaction_output_ptr_new()
* \returns CARDANO_TRANSACTION_SUCCESS | CARDANO_TRANSACTION_COIN_OUT_OF_BOUNDS
*/
cardano_transaction_error_t cardano_transaction_builder_add_input(cardano_transaction_builder *tb, cardano_txoptr *c_txo, uint64_t value);

/*!
* \brief This associate all the leftover values, if any to an output with the specified address.
*
* If the transaction is already consuming all inputs in its outputs (perfectly balanced),
* then it has no effect
*
* If there's not enough inputs value compared to the existing outputs, then a failure status is returned.
* If there's no way to "fit" the output policy in the transaction building, as the fee cannot cover
* the basic overhead, then a failure status is returned.
*
* Note: that the calculation is not done again if more inputs and outputs are added after this call,
* and in most typical cases this should be the last addition to the transaction.
*
* \param [in] tb the builder for the transaction
* \param [in] change_addr used for the change (leftover values) output 
* \returns 0 for success 0! for failure
*/
cardano_result cardano_transaction_builder_add_change_addr(cardano_transaction_builder *tb, cardano_address *change_addr);

/*!
* \brief Calculate the fee for the transaction with a linear algorithm
* \returns fee
*/
uint64_t cardano_transaction_builder_fee(cardano_transaction_builder *tb);

/*!
* struct for representing the sign in cardano_transaction_coin_diff_t
* \sa cardano_transaction_coin_diff
*/
typedef enum difference_type {
    DIFF_POSITIVE,
    DIFF_NEGATIVE,
    DIFF_ZERO,
} difference_type_t;

/*!
* struct for repressenting a balance returned with cardano_transaction_builder_balance and cardano_transaction_builder_balance_without_fees
* \sa cardano_transaction_builder_balance() cardano_transaction_builder_balance_without_fees()
*/
typedef struct cardano_transaction_coin_diff {
    difference_type_t sign;
    uint64_t value;
} cardano_transaction_coin_diff_t;

/*!
* \brief Deallocate the memory allocated with `cardano_transaction_builder_balance` and `cardano_transaction_builder_balance_without_fees`
* \sa cardano_transaction_builder_balance() cardano_transaction_builder_balance_without_fees()
*/
void cardano_transaction_balance_delete(cardano_transaction_coin_diff_t *balance);

/*!
* Try to return the differential between the outputs (including fees) and the inputs
* \param [in] tb the builder for the transaction
* \param [out] out a pointer to a cardano_transaction_coin_diff_t where: 
*   - (sign == DIFF_ZERO) means we have a balanced transaction where inputs === outputs
*   - (sign == DIFF_NEGATIVE) means (outputs+fees) > inputs. More inputs required.
*   - (sign == DIFF_POSITIVE) means inputs > (outputs+fees). 
*   .
* and the value field indicates the quantity (in -1 and 1 cases)
* Excessive input goes to larger fee.
* \returns CARDANO_TRANSACTION_SUCCESS | CARDANO_TRANSACTION_COIN_OUT_OF_BOUNDS if the total is too big
* \sa cardano_transaction_balance_delete()
*/
cardano_transaction_error_t cardano_transaction_builder_balance(cardano_transaction_builder *tb, cardano_transaction_coin_diff_t **out);

/*!
* Try to return the differential between the outputs (excluding fees) and the inputs
* \param [in] tb the builder for the transaction
* \param [out] out a pointer to a cardano_transaction_coin_diff_t where:
*   - (sign == DIFF_ZERO) means we have a balanced transaction where inputs === outputs
*   - (sign == DIFF_NEGATIVE) means (outputs) > inputs. More inputs required.
*   - (sign == DIFF_POSITIVE) means inputs > (outputs). 
*   .
* and the value field indicates the quantity (in -1 and 1 cases)
* \returns CARDANO_TRANSACTION_SUCCESS | CARDANO_TRANSACTION_COIN_OUT_OF_BOUNDS if the total is too big
* \sa cardano_transaction_balance_delete()
*/
cardano_transaction_error_t cardano_transaction_builder_balance_without_fees(cardano_transaction_builder *tb, cardano_transaction_coin_diff_t **out);

/*!
* Try to return the sum of the inputs
* \param [in] tb the builder for the transaction
* \param [out] output the sum
* \returns CARDANO_TRANSACTION_SUCCESS | CARDANO_TRANSACTION_COIN_OUT_OF_BOUNDS if the total is too big
*/
cardano_transaction_error_t cardano_transaction_builder_get_input_total(cardano_transaction_builder *tb, uint64_t *output);

/*!
* Try to return the sum of the outputs
* \param [in] tb the builder for the transaction
* \param [out] output the sum
* \returns CARDANO_TRANSACTION_SUCCESS | CARDANO_TRANSACTION_COIN_OUT_OF_BOUNDS if the total is too big
*/
cardano_transaction_error_t cardano_transaction_builder_get_output_total(cardano_transaction_builder *tb, uint64_t *output);

/*!
* \brief Get a transaction object
* \returns CARDANO_TRANSACTION_SUCCESS | CARDANO_TRANSACTION_NO_INPUT | CARDANO_TRANSACTION_NO_OUTPUT
*/
cardano_transaction_error_t cardano_transaction_builder_finalize(cardano_transaction_builder *tb, cardano_transaction **tx);
void cardano_transaction_delete(cardano_transaction *c_tx);

/*!
* \brief Take a transaction and create a working area for adding witnesses
*/
cardano_transaction_finalized * cardano_transaction_finalized_new(cardano_transaction *c_tx);
void cardano_transaction_finalized_delete(cardano_transaction_finalized *tf);

/*!
* Add a witness associated with the next input.
*
* Witness need to be added in the same order to the inputs, otherwise protocol level mismatch will happen, and the transaction will be rejected
* \param tf a transaction finalized 
* \param c_xprv
* \param protocol_magic
* \param c_txid
* \sa cardano_transaction_builder_new
* \returns CARDANO_TRANSACTION_SUCCESS | CARDANO_TRANSACTION_SIGNATURES_EXCEEDED
*/
cardano_transaction_error_t cardano_transaction_finalized_add_witness(cardano_transaction_finalized *tf, uint8_t c_xprv[96], uint32_t protocol_magic, uint8_t c_txid[32]);

/*!
* \brief A finalized transaction with the vector of witnesses
* \param tf a finalized transaction with witnesses
* \sa cardano_transaction_finalized_add_witness()
* \returns CARDANO_TRANSACTION_SUCCESS | CARDANO_TRANSACTION_SIGNATURE_MISMATCH | CARDANO_TRANSACTION_OVER_LIMIT
*/
cardano_transaction_error_t cardano_transaction_finalized_output(cardano_transaction_finalized *tf, cardano_signed_transaction **txaux);
void cardano_transaction_signed_delete(cardano_signed_transaction *txaux);

/****************/
/* Block */
/****************/

typedef struct cardano_block cardano_block;

/*! \brief Try to the decode a CBOR encoded block
* \param [in] bytes the block in its raw representation
* \param [in] size the size in bytes of the binary 
* \param [out] out_block a pointer to the block, you should call `cardano_block_delete` on this pointer
* \returns error if the binary format is wrong 
*/
cardano_result cardano_raw_block_decode(const uint8_t *bytes, size_t size, cardano_block **out_block);

/*! \brief delete memory allocated by `cardano_raw_block_decode`
*/
void cardano_block_delete(cardano_block *block);

/*! \brief Get references to the block transactions
* \param [in] block the corresponding block
* \param [out] out_array this array contains pointers that you can use for 
cardano_signed_transaction_get_inputs() and cardano_signed_transaction_get_outputs()
you should NOT call `cardano_transaction_signed_delete`() on any of this pointers.
You should call cardano_block_delete_transactions to free the memory allocated for this pointers
* \param [out] size the number of transactions
* \returns CARDANO_RESULT_ERROR if the block is a BoundaryBlock
* \sa cardano_signed_transaction_get_inputs()
* \sa cardano_signed_transaction_get_outputs()
* \sa cardano_block_delete_transactions()
*/
cardano_result cardano_block_get_transactions(
    cardano_block *block,
    cardano_signed_transaction *(*out_array[]),
    size_t *size
);

/*! \brief Release the memory allocated by cardano_block_get_transactions
*/
void cardano_block_delete_transactions(
    cardano_signed_transaction *transactions[], size_t size
);

/*!
* \struct cardano_block_header
* \brief Opaque handler for the Header of a Block
*
* Struct for working with block headers, which can be used to obtain the id of the respective and previous blocks
*/
typedef struct cardano_block_header cardano_block_header;

/*!
*\brief Get a copy of the given block's header
*\param [in] block
*\returns a pointer to a heap allocated copy of the header inside the block.
*You should call `cardano_block_header_delete()` to release the memory
*\relatesalso cardano_block_header
*/
cardano_block_header *cardano_block_get_header(cardano_block *block);

/*!
*\brief Decode a header in its cbor representation
*\param [in] bytes buffer
*\param [in] size size of the given buffer
*\param [out] out_header a pointer to the header
*You should call `cardano_block_header_delete()` to release the memory
*\relatesalso cardano_block_header
*/
cardano_result cardano_raw_block_header_decode(
    const uint8_t *bytes, size_t size, cardano_block_header **out_header);

/*!
*\brief Get the hash (as a string in hexaedecimal representation) of the previous header (which is the previous blockid)
*\param [in] header pointer to the header obtained with `cardano_block_get_header()` or `cardano_raw_block_header_decode()`
*\relatesalso cardano_block_header
*\sa `cardano_block_delete_hash()`
*/
char *cardano_block_header_previous_hash(cardano_block_header *header);

/*!
*\brief get the hash (in hexadecimal representation) of a given header, that can be used as the blockid
*\param [in] header
*\relatesalso cardano_block_header
*\sa `cardano_block_delete_hash()`
*/
char *cardano_block_header_compute_hash(cardano_block_header *header);

/*!
*\brief release the memory allocated with cardano_block_header_previous_hash 
*    or cardano_block_header_compute_hash
*\param [in] hash
*\relatesalso cardano_block_header_previous_hash
*\relatesalso cardano_block_header_compute_hash
*/
void cardano_block_delete_hash(char *hash);

/*!
*\brief release the memory allocated with `cardano_block_get_header` and `cardano_raw_block_header_decode`
*\param [in] hash
*\relatesalso cardano_block_header
*/
void cardano_block_header_delete(cardano_block_header *header);
char *create_rootkey(const char* mnemonics, const char* password);

#ifdef __cplusplus
}
#endif

#endif
