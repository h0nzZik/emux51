#include <stdlib.h>
#include <glib.h>
#include <stdio.h>
#include <lists.h>


dlist_t *list_first(dlist_t *list)
{
	while (list->prev){
		list=list->prev;
	}
	return list;
}


dlist_t *dlist_alloc(void (*f)(void *, void *), void *instance, void *data)
{
	dlist_t *new;
	printf("[emux51]\tallocating new dlist entry...\t");
	new=calloc(1, sizeof(dlist_t));
	if (new == NULL){
		printf("[failed]\n");
		return NULL;
	}
	printf("new dlist entry at %p\n", new);
	new->f=f;
	new->instance=instance;
	new->data=data;
	return new;
}

int dlist_count(dlist_t *list)
{
	int i=0;
	while(list) {
		i++;
		list=list->next;
	}
	return i;
}


dlist_t *dlist_link(dlist_t *list, dlist_t *new, unsigned dt)
{
//	printf("dlist_link, list:%p, new:%p\n", list, new);

	new->dt=dt;
	new->prev=NULL;
	new->next=NULL;

	while (list) {
		/*	prepend?	*/
		if (list->dt >= new->dt) {
			list->dt -= new->dt;
			new->next=list;
			new->prev=list->prev;
			if (new->prev)
				new->prev->next=new;
			list->prev=new;
			return list_first(new);
		}

		/*	no, append	*/
		new->dt -= list->dt;
		if (list->next == NULL) {
			list->next=new;
			new->prev=list;

			return list_first(new);
		}
		list=list->next;
	}

	/*	first element was NULL	*/
	return new;
}
void dlist_dump(dlist_t *list)
{
	printf("[emux51] staring dlist dump\n");
	while(list) {
		printf("list:%p\n\t->prev:\t%p\n\t->next:\t%p\n", list, list->prev, list->next);
		list=list->next;	
	}
}

void dlist_perform(dlist_t **first, unsigned steps)
{
	dlist_t *list;
	dlist_t *expired;
	int dump=0;


	dlist_t *next;

	list=*first;
	expired=list;

	/*	We must divide list into two.
		The first of these will contain entries with expired time,
		the second will be stored to as new list. 	*/
	while (list) {
		/*	end of first list	*/
		if (list->dt > steps) {
//			printf("%p->dt = %u - %u\n", list,list->dt, steps);
			list->dt -= steps;

			if (list->prev)
				list->prev->next=NULL;
			else
				expired=NULL;
			list->prev=NULL;

			break;
		}
		steps -= list->dt;
		list=list->next;
	}

	/*	new list	*/
	*first=list;

	if (dump) {
		printf("dumping new list:\n");
		dlist_dump(list);
		printf("dumping expired list:\n");
		dlist_dump(expired);	
	}

	/*	run 'expired' list	*/
	int i=0;
	while(expired) {
		if (dump)
			printf("executing %d -> %p\n", ++i, expired);
		next=expired->next;
		expired->f(expired->instance, expired->data);
		expired=next;
	}
}

void dlist_unlink(dlist_t **first, dlist_t *entry)
{
/*	printf("[emux51]\tunlinking node %p from list:\n"
		"\t->prev:\t%p\n\t->next:\t%p\n",
		entry, entry->prev, entry->next);
*/
	if (entry->prev)
		entry->prev->next=entry->next;
	else
		*first=entry->next;
	if (entry->next)
		entry->next->prev=entry->prev;


//	dlist_dump(*first);
}

