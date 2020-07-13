// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2018 Patrick Wildt <patrick@blueri.se>
 * Copyright (c) 2019 Linaro Limited, Author: AKASHI Takahiro
 */

#include <common.h>
#include <charset.h>
#include <efi_loader.h>
#include <image.h>
#include <hexdump.h>
#include <malloc.h>
#include <crypto/pkcs7_parser.h>
#include <linux/compat.h>
#include <linux/oid_registry.h>
#include <u-boot/rsa.h>
#include <u-boot/sha256.h>

const efi_guid_t efi_guid_image_security_database =
		EFI_IMAGE_SECURITY_DATABASE_GUID;
const efi_guid_t efi_guid_sha256 = EFI_CERT_SHA256_GUID;
const efi_guid_t efi_guid_cert_rsa2048 = EFI_CERT_RSA2048_GUID;
const efi_guid_t efi_guid_cert_x509 = EFI_CERT_X509_GUID;
const efi_guid_t efi_guid_cert_x509_sha256 = EFI_CERT_X509_SHA256_GUID;
const efi_guid_t efi_guid_cert_type_pkcs7 = EFI_CERT_TYPE_PKCS7_GUID;

#ifdef CONFIG_EFI_SECURE_BOOT

/**
 * efi_hash_regions - calculate a hash value
 * @regs:	Array of regions
 * @count:	Number of regions
 * @hash:	Pointer to a pointer to buffer holding a hash value
 * @size:	Size of buffer to be returned
 *
 * Calculate a sha256 value of @regs and return a value in @hash.
 *
 * Return:	true on success, false on error
 */
static bool efi_hash_regions(struct image_region *regs, int count,
			     void **hash, size_t *size)
{
	if (!*hash) {
		*hash = calloc(1, SHA256_SUM_LEN);
		if (!*hash) {
			EFI_PRINT("Out of memory\n");
			return false;
		}
	}
	if (size)
		*size = SHA256_SUM_LEN;

	hash_calculate("sha256", regs, count, *hash);
#ifdef DEBUG
	EFI_PRINT("hash calculated:\n");
	print_hex_dump("    ", DUMP_PREFIX_OFFSET, 16, 1,
		       *hash, SHA256_SUM_LEN, false);
#endif

	return true;
}

/**
 * efi_hash_msg_content - calculate a hash value of contentInfo
 * @msg:	Signature
 * @hash:	Pointer to a pointer to buffer holding a hash value
 * @size:	Size of buffer to be returned
 *
 * Calculate a sha256 value of contentInfo in @msg and return a value in @hash.
 *
 * Return:	true on success, false on error
 */
static bool efi_hash_msg_content(struct pkcs7_message *msg, void **hash,
				 size_t *size)
{
	struct image_region regtmp;

	regtmp.data = msg->data;
	regtmp.size = msg->data_len;

	return efi_hash_regions(&regtmp, 1, hash, size);
}

/**
 * efi_signature_verify - verify a signature with a certificate
 * @regs:		List of regions to be authenticated
 * @signed_info:	Pointer to PKCS7's signed_info
 * @cert:		x509 certificate
 *
 * Signature pointed to by @signed_info against image pointed to by @regs
 * is verified by a certificate pointed to by @cert.
 * @signed_info holds a signature, including a message digest which is to be
 * compared with a hash value calculated from @regs.
 *
 * Return:	true if signature is verified, false if not
 */
static bool efi_signature_verify(struct efi_image_regions *regs,
				 struct pkcs7_message *msg,
				 struct pkcs7_signed_info *ps_info,
				 struct x509_certificate *cert)
{
	struct image_sign_info info;
	struct image_region regtmp[2];
	void *hash;
	size_t size;
	char c;
	bool verified;

	EFI_PRINT("%s: Enter, %p, %p, %p(issuer: %s, subject: %s)\n", __func__,
		  regs, ps_info, cert, cert->issuer, cert->subject);

	verified = false;

	memset(&info, '\0', sizeof(info));
	info.padding = image_get_padding_algo("pkcs-1.5");
	/*
	 * Note: image_get_[checksum|crypto]_algo takes an string
	 * argument like "<checksum>,<crypto>"
	 * TODO: support other hash algorithms
	 */
	if (!strcmp(ps_info->sig->hash_algo, "sha1")) {
		info.checksum = image_get_checksum_algo("sha1,rsa2048");
		info.name = "sha1,rsa2048";
	} else if (!strcmp(ps_info->sig->hash_algo, "sha256")) {
		info.checksum = image_get_checksum_algo("sha256,rsa2048");
		info.name = "sha256,rsa2048";
	} else {
		EFI_PRINT("unknown msg digest algo: %s\n",
			  ps_info->sig->hash_algo);
		goto out;
	}
	info.crypto = image_get_crypto_algo(info.name);

	info.key = cert->pub->key;
	info.keylen = cert->pub->keylen;

	/* verify signature */
	EFI_PRINT("%s: crypto: %s, signature len:%x\n", __func__,
		  info.name, ps_info->sig->s_size);
	if (ps_info->aa_set & (1UL << sinfo_has_message_digest)) {
		EFI_PRINT("%s: RSA verify authentication attribute\n",
			  __func__);
		/*
		 * NOTE: This path will be executed only for
		 * PE image authentication
		 */

		/* check if hash matches digest first */
		EFI_PRINT("checking msg digest first, len:0x%x\n",
			  ps_info->msgdigest_len);

#ifdef DEBUG
		EFI_PRINT("hash in database:\n");
		print_hex_dump("    ", DUMP_PREFIX_OFFSET, 16, 1,
			       ps_info->msgdigest, ps_info->msgdigest_len,
			       false);
#endif
		/* against contentInfo first */
		hash = NULL;
		if ((msg->data && efi_hash_msg_content(msg, &hash, &size)) ||
				/* for signed image */
		    efi_hash_regions(regs->reg, regs->num, &hash, &size)) {
				/* for authenticated variable */
			if (ps_info->msgdigest_len != size ||
			    memcmp(hash, ps_info->msgdigest, size)) {
				EFI_PRINT("Digest doesn't match\n");
				free(hash);
				goto out;
			}

			free(hash);
		} else {
			EFI_PRINT("Digesting image failed\n");
			goto out;
		}

		/* against digest */
		c = 0x31;
		regtmp[0].data = &c;
		regtmp[0].size = 1;
		regtmp[1].data = ps_info->authattrs;
		regtmp[1].size = ps_info->authattrs_len;

		if (!rsa_verify(&info, regtmp, 2,
				ps_info->sig->s, ps_info->sig->s_size))
			verified = true;
	} else {
		EFI_PRINT("%s: RSA verify content data\n", __func__);
		/* against all data */
		if (!rsa_verify(&info, regs->reg, regs->num,
				ps_info->sig->s, ps_info->sig->s_size))
			verified = true;
	}

out:
	EFI_PRINT("%s: Exit, verified: %d\n", __func__, verified);
	return verified;
}

/**
 * efi_signature_lookup_digest - search for an image's digest in sigdb
 * @regs:	List of regions to be authenticated
 * @db:		Signature database for trusted certificates
 *
 * A message digest of image pointed to by @regs is calculated and
 * its hash value is compared to entries in signature database pointed
 * to by @db.
 *
 * Return:	true if found, false if not
 */
bool efi_signature_lookup_digest(struct efi_image_regions *regs,
				 struct efi_signature_store *db)
{
	struct efi_signature_store *siglist;
	struct efi_sig_data *sig_data;
	void *hash = NULL;
	size_t size = 0;
	bool found = false;

	EFI_PRINT("%s: Enter, %p, %p\n", __func__, regs, db);

	if (!regs || !db || !db->sig_data_list)
		goto out;

	for (siglist = db; siglist; siglist = siglist->next) {
		/* TODO: support other hash algorithms */
		if (guidcmp(&siglist->sig_type, &efi_guid_sha256)) {
			EFI_PRINT("Digest algorithm is not supported: %pUl\n",
				  &siglist->sig_type);
			break;
		}

		if (!efi_hash_regions(regs->reg, regs->num, &hash, &size)) {
			EFI_PRINT("Digesting an image failed\n");
			break;
		}

		for (sig_data = siglist->sig_data_list; sig_data;
		     sig_data = sig_data->next) {
#ifdef DEBUG
			EFI_PRINT("Msg digest in database:\n");
			print_hex_dump("    ", DUMP_PREFIX_OFFSET, 16, 1,
				       sig_data->data, sig_data->size, false);
#endif
			if (sig_data->size == size &&
			    !memcmp(sig_data->data, hash, size)) {
				found = true;
				free(hash);
				goto out;
			}
		}

		free(hash);
		hash = NULL;
	}

out:
	EFI_PRINT("%s: Exit, found: %d\n", __func__, found);
	return found;
}

/**
 * efi_signature_verify_with_list - verify a signature with signature list
 * @regs:		List of regions to be authenticated
 * @msg:		Signature
 * @signed_info:	Pointer to PKCS7's signed_info
 * @siglist:		Signature list for certificates
 * @valid_cert:		x509 certificate that verifies this signature
 *
 * Signature pointed to by @signed_info against image pointed to by @regs
 * is verified by signature list pointed to by @siglist.
 * Signature database is a simple concatenation of one or more
 * signature list(s).
 *
 * Return:	true if signature is verified, false if not
 */
static
bool efi_signature_verify_with_list(struct efi_image_regions *regs,
				    struct pkcs7_message *msg,
				    struct pkcs7_signed_info *signed_info,
				    struct efi_signature_store *siglist,
				    struct x509_certificate **valid_cert)
{
	struct x509_certificate *cert;
	struct efi_sig_data *sig_data;
	bool verified = false;

	EFI_PRINT("%s: Enter, %p, %p, %p, %p\n", __func__,
		  regs, signed_info, siglist, valid_cert);

	if (guidcmp(&siglist->sig_type, &efi_guid_cert_x509)) {
		EFI_PRINT("Signature type is not supported: %pUl\n",
			  &siglist->sig_type);
		goto out;
	}

	/* go through the list */
	for (sig_data = siglist->sig_data_list; sig_data;
	     sig_data = sig_data->next) {
		/* TODO: support owner check based on policy */

		cert = x509_cert_parse(sig_data->data, sig_data->size);
		if (IS_ERR(cert)) {
			EFI_PRINT("Parsing x509 certificate failed\n");
			goto out;
		}

		verified = efi_signature_verify(regs, msg, signed_info, cert);

		if (verified) {
			if (valid_cert)
				*valid_cert = cert;
			else
				x509_free_certificate(cert);
			break;
		}
		x509_free_certificate(cert);
	}

out:
	EFI_PRINT("%s: Exit, verified: %d\n", __func__, verified);
	return verified;
}

/**
 * efi_signature_check_revocation - check revocation with dbx
 * @sinfo:	Signer's info
 * @cert:	x509 certificate
 * @dbx:	Revocation signature database
 *
 * Search revocation signature database pointed to by @dbx and find
 * an entry matching to certificate pointed to by @cert.
 *
 * While this entry contains revocation time, we don't support timestamp
 * protocol at this time and any image will be unconditionally revoked
 * when this match occurs.
 *
 * Return:	true if check passed, false otherwise.
 */
static bool efi_signature_check_revocation(struct pkcs7_signed_info *sinfo,
					   struct x509_certificate *cert,
					   struct efi_signature_store *dbx)
{
	struct efi_signature_store *siglist;
	struct efi_sig_data *sig_data;
	struct image_region reg[1];
	void *hash = NULL;
	size_t size = 0;
	time64_t revoc_time;
	bool revoked = false;

	EFI_PRINT("%s: Enter, %p, %p, %p\n", __func__, sinfo, cert, dbx);

	if (!sinfo || !cert || !dbx || !dbx->sig_data_list)
		goto out;

	EFI_PRINT("Checking revocation against %s\n", cert->subject);
	for (siglist = dbx; siglist; siglist = siglist->next) {
		if (guidcmp(&siglist->sig_type, &efi_guid_cert_x509_sha256))
			continue;

		/* calculate hash of TBSCertificate */
		reg[0].data = cert->tbs;
		reg[0].size = cert->tbs_size;
		if (!efi_hash_regions(reg, 1, &hash, &size))
			goto out;

		for (sig_data = siglist->sig_data_list; sig_data;
		     sig_data = sig_data->next) {
			/*
			 * struct efi_cert_x509_sha256 {
			 *	u8 tbs_hash[256/8];
			 *	time64_t revocation_time;
			 * };
			 */
#ifdef DEBUG
			if (sig_data->size >= size) {
				EFI_PRINT("hash in db:\n");
				print_hex_dump("    ", DUMP_PREFIX_OFFSET,
					       16, 1,
					       sig_data->data, size, false);
			}
#endif
			if ((sig_data->size < size + sizeof(time64_t)) ||
			    memcmp(sig_data->data, hash, size))
				continue;

			memcpy(&revoc_time, sig_data->data + size,
			       sizeof(revoc_time));
			EFI_PRINT("revocation time: 0x%llx\n", revoc_time);
			/*
			 * TODO: compare signing timestamp in sinfo
			 * with revocation time
			 */

			revoked = true;
			free(hash);
			goto out;
		}
		free(hash);
		hash = NULL;
	}
out:
	EFI_PRINT("%s: Exit, revoked: %d\n", __func__, revoked);
	return !revoked;
}

/**
 * efi_signature_verify_one - verify signatures with database
 * @regs:	List of regions to be authenticated
 * @msg:	Signature
 * @db:		Signature database
 *
 * All the signature pointed to by @msg against image pointed to by @regs
 * will be verified by signature database pointed to by @db.
 *
 * Return:	true if verification for one of signatures passed, false
 *		otherwise
 */
bool efi_signature_verify_one(struct efi_image_regions *regs,
			      struct pkcs7_message *msg,
			      struct efi_signature_store *db)
{
	struct pkcs7_signed_info *sinfo;
	struct efi_signature_store *siglist;
	struct x509_certificate *cert;
	bool verified = false;

	EFI_PRINT("%s: Enter, %p, %p, %p\n", __func__, regs, msg, db);

	if (!db)
		goto out;

	if (!db->sig_data_list)
		goto out;

	EFI_PRINT("%s: Verify signed image with db\n", __func__);
	for (sinfo = msg->signed_infos; sinfo; sinfo = sinfo->next) {
		EFI_PRINT("Signed Info: digest algo: %s, pkey algo: %s\n",
			  sinfo->sig->hash_algo, sinfo->sig->pkey_algo);

		for (siglist = db; siglist; siglist = siglist->next)
			if (efi_signature_verify_with_list(regs, msg, sinfo,
							   siglist, &cert)) {
				verified = true;
				goto out;
			}
		EFI_PRINT("Valid certificate not in \"db\"\n");
	}

out:
	EFI_PRINT("%s: Exit, verified: %d\n", __func__, verified);
	return verified;
}

/**
 * efi_signature_verify_with_sigdb - verify signatures with db and dbx
 * @regs:	List of regions to be authenticated
 * @msg:	Signature
 * @db:		Signature database for trusted certificates
 * @dbx:	Revocation signature database
 *
 * All the signature pointed to by @msg against image pointed to by @regs
 * will be verified by signature database pointed to by @db and @dbx.
 *
 * Return:	true if verification for all signatures passed, false otherwise
 */
bool efi_signature_verify_with_sigdb(struct efi_image_regions *regs,
				     struct pkcs7_message *msg,
				     struct efi_signature_store *db,
				     struct efi_signature_store *dbx)
{
	struct pkcs7_signed_info *info;
	struct efi_signature_store *siglist;
	struct x509_certificate *cert;
	bool verified = false;

	EFI_PRINT("%s: Enter, %p, %p, %p, %p\n", __func__, regs, msg, db, dbx);

	if (!regs || !msg || !db || !db->sig_data_list)
		goto out;

	for (info = msg->signed_infos; info; info = info->next) {
		EFI_PRINT("Signed Info: digest algo: %s, pkey algo: %s\n",
			  info->sig->hash_algo, info->sig->pkey_algo);

		for (siglist = db; siglist; siglist = siglist->next) {
			if (efi_signature_verify_with_list(regs, msg, info,
							   siglist, &cert))
				break;
		}
		if (!siglist) {
			EFI_PRINT("Valid certificate not in \"db\"\n");
			goto out;
		}

		if (!dbx || efi_signature_check_revocation(info, cert, dbx))
			continue;

		EFI_PRINT("Certificate in \"dbx\"\n");
		goto out;
	}
	verified = true;

out:
	EFI_PRINT("%s: Exit, verified: %d\n", __func__, verified);
	return verified;
}

/**
 * efi_signature_check_signers - check revocation against all signers with dbx
 * @msg:	Signature
 * @dbx:	Revocation signature database
 *
 * Determine if none of signers' certificates in @msg are revoked
 * by signature database pointed to by @dbx.
 *
 * Return:	true if all signers passed, false otherwise.
 */
bool efi_signature_check_signers(struct pkcs7_message *msg,
				 struct efi_signature_store *dbx)
{
	struct pkcs7_signed_info *sinfo;
	bool revoked = false;

	EFI_PRINT("%s: Enter, %p, %p\n", __func__, msg, dbx);

	if (!msg || !dbx)
		goto out;

	for (sinfo = msg->signed_infos; sinfo; sinfo = sinfo->next) {
		if (sinfo->signer &&
		    !efi_signature_check_revocation(sinfo, sinfo->signer,
						    dbx)) {
			revoked = true;
			break;
		}
	}
out:
	EFI_PRINT("%s: Exit, revoked: %d\n", __func__, revoked);
	return !revoked;
}

/**
 * efi_image_region_add() - add an entry of region
 * @regs:	Pointer to array of regions
 * @start:	Start address of region (included)
 * @end:	End address of region (excluded)
 * @nocheck:	flag against overlapped regions
 *
 * Take one entry of region [@start, @end[ and insert it into the list.
 *
 * * If @nocheck is false, the list will be sorted ascending by address.
 *   Overlapping entries will not be allowed.
 *
 * * If @nocheck is true, the list will be sorted ascending by sequence
 *   of adding the entries. Overlapping is allowed.
 *
 * Return:	status code
 */
efi_status_t efi_image_region_add(struct efi_image_regions *regs,
				  const void *start, const void *end,
				  int nocheck)
{
	struct image_region *reg;
	int i, j;

	if (regs->num >= regs->max) {
		EFI_PRINT("%s: no more room for regions\n", __func__);
		return EFI_OUT_OF_RESOURCES;
	}

	if (end < start)
		return EFI_INVALID_PARAMETER;

	for (i = 0; i < regs->num; i++) {
		reg = &regs->reg[i];
		if (nocheck)
			continue;

		/* new data after registered region */
		if (start >= reg->data + reg->size)
			continue;

		/* new data preceding registered region */
		if (end <= reg->data) {
			for (j = regs->num - 1; j >= i; j--)
				memcpy(&regs->reg[j + 1], &regs->reg[j],
				       sizeof(*reg));
			break;
		}

		/* new data overlapping registered region */
		EFI_PRINT("%s: new region already part of another\n", __func__);
		return EFI_INVALID_PARAMETER;
	}

	reg = &regs->reg[i];
	reg->data = start;
	reg->size = end - start;
	regs->num++;

	return EFI_SUCCESS;
}

/**
 * efi_sigstore_free - free signature store
 * @sigstore:	Pointer to signature store structure
 *
 * Feee all the memories held in signature store and itself,
 * which were allocated by efi_sigstore_parse_sigdb().
 */
void efi_sigstore_free(struct efi_signature_store *sigstore)
{
	struct efi_signature_store *sigstore_next;
	struct efi_sig_data *sig_data, *sig_data_next;

	while (sigstore) {
		sigstore_next = sigstore->next;

		sig_data = sigstore->sig_data_list;
		while (sig_data) {
			sig_data_next = sig_data->next;
			free(sig_data->data);
			free(sig_data);
			sig_data = sig_data_next;
		}

		free(sigstore);
		sigstore = sigstore_next;
	}
}

/**
 * efi_sigstore_parse_siglist - parse a signature list
 * @name:	Pointer to signature list
 *
 * Parse signature list and instantiate a signature store structure.
 * Signature database is a simple concatenation of one or more
 * signature list(s).
 *
 * Return:	Pointer to signature store on success, NULL on error
 */
static struct efi_signature_store *
efi_sigstore_parse_siglist(struct efi_signature_list *esl)
{
	struct efi_signature_store *siglist = NULL;
	struct efi_sig_data *sig_data, *sig_data_next;
	struct efi_signature_data *esd;
	size_t left;

	/*
	 * UEFI specification defines certificate types:
	 *   for non-signed images,
	 *	EFI_CERT_SHA256_GUID
	 *	EFI_CERT_RSA2048_GUID
	 *	EFI_CERT_RSA2048_SHA256_GUID
	 *	EFI_CERT_SHA1_GUID
	 *	EFI_CERT_RSA2048_SHA_GUID
	 *	EFI_CERT_SHA224_GUID
	 *	EFI_CERT_SHA384_GUID
	 *	EFI_CERT_SHA512_GUID
	 *
	 *   for signed images,
	 *	EFI_CERT_X509_GUID
	 *	NOTE: Each certificate will normally be in a separate
	 *	EFI_SIGNATURE_LIST as the size may vary depending on
	 *	its algo's.
	 *
	 *   for timestamp revocation of certificate,
	 *	EFI_CERT_X509_SHA512_GUID
	 *	EFI_CERT_X509_SHA256_GUID
	 *	EFI_CERT_X509_SHA384_GUID
	 */

	if (esl->signature_list_size
			<= (sizeof(*esl) + esl->signature_header_size)) {
		EFI_PRINT("Siglist in wrong format\n");
		return NULL;
	}

	/* Create a head */
	siglist = calloc(sizeof(*siglist), 1);
	if (!siglist) {
		EFI_PRINT("Out of memory\n");
		goto err;
	}
	memcpy(&siglist->sig_type, &esl->signature_type, sizeof(efi_guid_t));

	/* Go through the list */
	sig_data_next = NULL;
	left = esl->signature_list_size
			- (sizeof(*esl) + esl->signature_header_size);
	esd = (struct efi_signature_data *)
			((u8 *)esl + sizeof(*esl) + esl->signature_header_size);

	while (left > 0) {
		/* Signature must exist if there is remaining data. */
		if (left < esl->signature_size) {
			EFI_PRINT("Certificate is too small\n");
			goto err;
		}

		sig_data = calloc(esl->signature_size
					- sizeof(esd->signature_owner), 1);
		if (!sig_data) {
			EFI_PRINT("Out of memory\n");
			goto err;
		}

		/* Append signature data */
		memcpy(&sig_data->owner, &esd->signature_owner,
		       sizeof(efi_guid_t));
		sig_data->size = esl->signature_size
					- sizeof(esd->signature_owner);
		sig_data->data = malloc(sig_data->size);
		if (!sig_data->data) {
			EFI_PRINT("Out of memory\n");
			goto err;
		}
		memcpy(sig_data->data, esd->signature_data, sig_data->size);

		sig_data->next = sig_data_next;
		sig_data_next = sig_data;

		/* Next */
		esd = (struct efi_signature_data *)
				((u8 *)esd + esl->signature_size);
		left -= esl->signature_size;
	}
	siglist->sig_data_list = sig_data_next;

	return siglist;

err:
	efi_sigstore_free(siglist);

	return NULL;
}

/**
 * efi_sigstore_parse_sigdb - parse a signature database variable
 * @name:	Variable's name
 *
 * Read in a value of signature database variable pointed to by
 * @name, parse it and instantiate a signature store structure.
 *
 * Return:	Pointer to signature store on success, NULL on error
 */
struct efi_signature_store *efi_sigstore_parse_sigdb(u16 *name)
{
	struct efi_signature_store *sigstore = NULL, *siglist;
	struct efi_signature_list *esl;
	const efi_guid_t *vendor;
	void *db;
	efi_uintn_t db_size;
	efi_status_t ret;

	if (!u16_strcmp(name, L"PK") || !u16_strcmp(name, L"KEK")) {
		vendor = &efi_global_variable_guid;
	} else if (!u16_strcmp(name, L"db") || !u16_strcmp(name, L"dbx")) {
		vendor = &efi_guid_image_security_database;
	} else {
		EFI_PRINT("unknown signature database, %ls\n", name);
		return NULL;
	}

	/* retrieve variable data */
	db_size = 0;
	ret = EFI_CALL(efi_get_variable(name, vendor, NULL, &db_size, NULL));
	if (ret == EFI_NOT_FOUND) {
		EFI_PRINT("variable, %ls, not found\n", name);
		sigstore = calloc(sizeof(*sigstore), 1);
		return sigstore;
	} else if (ret != EFI_BUFFER_TOO_SMALL) {
		EFI_PRINT("Getting variable, %ls, failed\n", name);
		return NULL;
	}

	db = malloc(db_size);
	if (!db) {
		EFI_PRINT("Out of memory\n");
		return NULL;
	}

	ret = EFI_CALL(efi_get_variable(name, vendor, NULL, &db_size, db));
	if (ret != EFI_SUCCESS) {
		EFI_PRINT("Getting variable, %ls, failed\n", name);
		goto err;
	}

	/* Parse siglist list */
	esl = db;
	while (db_size > 0) {
		/* List must exist if there is remaining data. */
		if (db_size < sizeof(*esl)) {
			EFI_PRINT("variable, %ls, in wrong format\n", name);
			goto err;
		}

		if (db_size < esl->signature_list_size) {
			EFI_PRINT("variable, %ls, in wrong format\n", name);
			goto err;
		}

		/* Parse a single siglist. */
		siglist = efi_sigstore_parse_siglist(esl);
		if (!siglist) {
			EFI_PRINT("Parsing signature list of %ls failed\n",
				  name);
			goto err;
		}

		/* Append siglist */
		siglist->next = sigstore;
		sigstore = siglist;

		/* Next */
		db_size -= esl->signature_list_size;
		esl = (void *)esl + esl->signature_list_size;
	}
	free(db);

	return sigstore;

err:
	efi_sigstore_free(sigstore);
	free(db);

	return NULL;
}
#endif /* CONFIG_EFI_SECURE_BOOT */
