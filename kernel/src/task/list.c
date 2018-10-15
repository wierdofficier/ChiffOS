#include <list.h>
#include <heapmngr.h>
#include <types.h>
ring_t* ring_list_create()
{
    ring_t* ring = valloc(sizeof(ring_t));
    return ring;
}

void ring_list_insert(ring_t* ring, void* data)
{
    if(!ring->begin) 
    {
        ring->begin = valloc(sizeof(element_t));
        ring->current = valloc(sizeof(element_t));
        ring->current->next = ring->current;
        ring->current->data = data;

    }
    else
    {
        element_t* item = valloc(sizeof(element_t));
        item->data = data;
        item->next = ring->current->next;
        ring->current->next = item;
	 
    }
}

bool ring_list_delete_first(ring_t* ring, void* data)
{
    element_t* current = ring->current;
    if(!ring->begin) return false;
    do
    {
        if (current->next->data == data) 
        {
            element_t* temp = current->next;
            
            if(temp == ring->begin) 
				ring->begin = ring->begin->next;
            if(temp == ring->current) 
				ring->current = ring->current->next;
				
            current->next = current->next->next;
            return true;
        }
        current = current->next;
    } while (current != ring->current);
    return false;
}



