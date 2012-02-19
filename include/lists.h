#ifndef LISTS_H
#define LISTS_H

/*	delta list type	*/
typedef struct DLIST {
	struct DLIST *prev;
	struct DLIST *next;
	unsigned dt;
	/* ... */
	void (*f)(void *instance, void *data);
	void *instance;
	void *data;
} dlist_t;


dlist_t *list_first(dlist_t *list);

dlist_t *dlist_link(dlist_t *list, dlist_t *new, unsigned dt);
void dlist_unlink(dlist_t **first, dlist_t *entry);

dlist_t *dlist_alloc(void (*f)(void *, void *), void *instance, void *data);
void dlist_perform(dlist_t **first, unsigned steps);

/*
typedef struct LLIST {
	struct LLIST *next;
	void (*f)(void *instance, void *data);
	void *instance;
	void *data;
} llist_t;
*/


#endif
