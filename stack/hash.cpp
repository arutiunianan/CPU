#include "hash.h"

Hash_t Hash(const void* arr, int len)
{
	Hash_t hash = 0;
	for (int i = 0; i < len; ++i) {
		hash += ((char*)arr)[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash;
}