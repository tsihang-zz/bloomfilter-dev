/*   oryx_cbf.c
 *   Created by TSIHANG <qh_soledadboy@sina.com>>
 *   12 July, 2017
 *   Personal.Q
 */

#include "oryx.h"
#include "oryx_cbf.h"

static int cbf_max_bytes = 10;
static const int cbf_count_bits = 4;

static __oryx_always_inline__
uint32_t oryx_fnv_hash (char* str, unsigned int len)  
{  

	const unsigned int fnv_prime = 0x811C9DC5;  
	unsigned int hash      = 0;  
	unsigned int i         = 0;  

	for(i = 0; i < len; str++, i++) {
		hash *= fnv_prime;
		hash ^= (*str);
	}
	
	return hash;
}  

static __oryx_always_inline__
uint32_t oryx_fnv1_hash (char* str, unsigned int len)  
{  

	const unsigned int fnv_prime = 16777619;  
	unsigned int hash      = 2166136261L;  
	unsigned int i         = 0;  

	for(i = 0; i < len; str++, i++) {
		hash *= fnv_prime;
		hash ^= (*str);
	}  
	
	hash += hash << 13;
	hash ^= hash >> 7;
	hash += hash << 3;
	hash ^= hash >> 17;
	hash += hash << 5;
	
	return hash;  
}  


static __oryx_always_inline__
uint32_t oryx_sdbm_hash (char* str, unsigned int len)  
{

	unsigned int hash = 0;
	unsigned int i    = 0;
	
	for(i = 0; i < len; str++, i++) {
		hash = (*str) + (hash << 6) + (hash << 16) - hash;
	}

	return hash;  
}  

static __oryx_always_inline__
uint32_t oryx_bkdr_hash (char* str, unsigned int len)  
{

	unsigned int seed = 131; /* 31 131 1313 13131 131313 etc.. */  
	unsigned int hash = 0;  
	unsigned int i    = 0;  

	for(i = 0; i < len; str++, i++) {  
		hash = (hash * seed) + (*str);  
	}
	
	return hash;  
}  

struct oryx_cbf_t *cbf_new (const char *bf_desc, int bf_max_objs)
{

	struct oryx_cbf_t *bf = NULL;

	bf = malloc (sizeof (struct oryx_cbf_t));
	if (likely (bf)) {
		bf->bf_desc = strdup (bf_desc);

		bf->max = bf_max_objs;

		HASH_INIT(&bf->hash[0], oryx_fnv_hash);
		HASH_INIT(&bf->hash[1], oryx_fnv1_hash);
		HASH_INIT(&bf->hash[2], oryx_sdbm_hash);
		HASH_INIT(&bf->hash[3], oryx_bkdr_hash);

		bf->k = 4;

		bf->nelmts = cbf_max_bytes;
		bf->m = malloc (bf->nelmts);
		memset (bf->m, 0, bf->nelmts);
	}
	

	return bf;
}

static __oryx_always_inline__
int cbf_at (struct oryx_cbf_t *bf, uint32_t h)
{
	uint32_t max_bits = bf->nelmts * 8;
	return h % max_bits;
}

static __oryx_always_inline__
void cbf_dump_byte (int index, uint8_t val)
{

	int j = 0;
	uint8_t b;
	char *y = CONSOLE_PRINT_CLOR_LYELLOW;
	char *r = CONSOLE_PRINT_CLOR_LWHITE;

	val ? printf ("Byte%s[%010d] --> "CONSOLE_PRINT_CLOR_FIN"", r, index) :\
		printf ("Byte[%010d] --> ", index);

	for (j = 0; j < 8; j ++) {
		b = ((val >> (7 - j)) & 0x01);
		b ? printf ("%s%d "CONSOLE_PRINT_CLOR_FIN"", y, b) : printf ("%d ", b);
	}

	printf ("\n");
}

void cbf_dump_array (struct oryx_cbf_t *bf)
{

	int i = 0;
	uint8_t *v = (uint8_t *)bf->m;

	printf ("\n\nTotal %d elements\n", bf->nelmts);
	for (i = 0; i < bf->nelmts; i ++) {
		cbf_dump_byte (i, v[i]);
	}

	printf ("\n");
}

static __oryx_always_inline__
void cbf_set (uint8_t *array, uint32_t offset) 
{

	uint8_t byte = 0;
	uint32_t offset_byte;
	uint32_t offset_bit;
	
	offset_byte = offset / 8;
	offset_bit  = offset % 8;

	byte = array[offset_byte];
	byte |= (1 << offset_bit);
	array[offset_byte] = byte;

	//bf_dump_byte (offset_byte, array[offset_byte]);
}

static __oryx_always_inline__
int cbf_get (uint8_t *array, uint32_t offset) 
{

	uint8_t byte = 0;
	uint32_t offset_byte;
	uint32_t offset_bit;
	
	offset_byte = offset / 8;
	offset_bit  = offset % 8;

	byte = array[offset_byte];

	return (byte >> offset_bit) & 0x01;
}

int cbf_add (struct oryx_cbf_t *bf, void *data, size_t s)
{

	uint32_t h;
	int i = 0;

	if (unlikely (!data)) return -1;
	
	for (i = 0; i < bf->k; i ++) {

		h = HASH_FUNC(bf, i)(data, s);
		//printf ("h(%s)=%u, @ offset (%d, %d) %d\n", HASH_DESC(bf, i), h, bf_at(bf, h) / 8, bf_at(bf, h) % 8, bf_at(bf, h));
		cbf_set ((uint8_t *)bf->m, cbf_at(bf, h));
	}

	return 0;
}

int cbf_query (struct oryx_cbf_t *bf, void *data, size_t s)
{

	uint32_t h;
	int i = 0;
	int result = 1;
	
	for (i = 0; i < bf->k; i ++) {

		if (!result) return 0;

		h = HASH_FUNC(bf, i)(data, s);
		result = cbf_get ((uint8_t *)bf->m, cbf_at(bf, h));
		/* printf ("result = %d,   h(%s)=%u, @ offset (%d, %d) %d\n", result
			HASH_DESC(bf, i), h, bf_at(bf, h) / 8, bf_at(bf, h) % 8, bf_at(bf, h)); */
	}

	return 1;
}


/** Random value generator. */
static __oryx_always_inline__ 
uint32_t next_rand_ (uint32_t *p)
{
	uint32_t seed = *p;

	seed = 1103515145 * seed + 12345;
	*p = seed;

	return seed;
}

static __oryx_always_inline__
void random_string_generate (char *key, size_t s)
{

	int i = 0;
	uint32_t rand = random();
	char *base = "abcdefghijklmnopqrstuvwxyz0123456789";
	
	next_rand_ (&rand);
	
	for (i = 0; i < (int)(s - 1); i ++) {
		int offset = rand % strlen(base);
		key[i] = base[offset];
		next_rand_ (&rand);
	}
}
 
