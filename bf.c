#include "oryx.h"
#include "oryx_bf.h"
//#include "oryx_cbf.h"


//#define random_test
 int main (void)
 {

 	printf ("\n\n");
	
 	int i = 0;
	int retval;
	struct oryx_bf_t *bf = bf_new ("test", 2);

#ifdef random_test

	char key[32] = {"hello"};

	for (i = 0; i < 1; i ++) {
		//random_string_generate (key, 32);
		bf_add (bf, key, 32);
		retval = bf_query (bf, key, 32);
		printf ("\"%10s\" %10s BloomFilter.\n", key, retval ? "maybe in" : "not in");
	}
	
#else

	#define MAX_KEYS 10

	/** Keyword. */
	char *key[MAX_KEYS] = {
		"Hello",
		"The",
		"Crule",
		"World",
		NULL,
		NULL};

	char key_out[32] = {"hello"};
	
	/** Add to Bloom Filter. */
	for (i = 0; i < MAX_KEYS; i ++) 
		bf_add (bf, key[i], 32);

	/** Query from Bloom Filter. */
	for (i = 0; i < MAX_KEYS; i ++) {
		if (!key[i]) continue;
		retval = bf_query (bf, key[i], 32);
		printf ("%10s %10s BloomFilter.\n", key[i], retval ? "maybe in" : "not in");
	}

	retval = bf_query (bf, key_out, 32);
	printf ("%10s %10s BloomFilter.\n", key_out, retval ? "maybe in" : "not in");

#endif	

	bf_dump_array (bf);
	
	return 0;
 }


