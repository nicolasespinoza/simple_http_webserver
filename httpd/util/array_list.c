#define _GNU_SOURCE

#include "array_list.h"
#include <stdlib.h>
#include <string.h>

struct arraylist* array_list_new(size_t data_type_size) {
    struct arraylist* list;
    list = malloc(sizeof(struct arraylist));
    list->data_type_size = data_type_size;
    list->item_capacity = 1;
    list->number_of_items = 0;
    list->data = malloc(data_type_size * (list->item_capacity));

    return list;
}

void array_list_add_to_end(struct arraylist* list, void* item) {
    int post_add_list_size = list->number_of_items + 1;
    if (post_add_list_size > list->item_capacity) { // resizing
        int resize_amount = list->item_capacity * 2;
        list->item_capacity = resize_amount;
        list->data = realloc(list->data, resize_amount * list->data_type_size);
    }
    list->data[list->number_of_items] = item; // NOTE: there is no copying done here
    list->number_of_items += 1;
}

void* array_list_get_item(struct arraylist* list, int index) {
    if (index >= list->number_of_items) {
//        printf("Index %d out of bounds for array list size %d\n", index, list->number_of_items);
        return NULL;
    }
    return list->data[index];
}

void array_list_cleanup(struct arraylist* list) {
    for (int i = 0; i < list->number_of_items; i++) {
        free(list->data[i]);
    }
    free(list->data);
    free(list);
}