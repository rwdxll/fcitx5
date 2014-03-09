#include "list.h"

static int fcitx_list_cmp_helper(const void* a, const void* b, void* data)
{
    FcitxCompareFunc compare = data;
    return compare(a, b);
}

FcitxListHead* fcitx_list_sorted_merge(FcitxListHead* heada, FcitxListHead* enda, FcitxListHead* headb, FcitxListHead* endb, FcitxListHead** end, size_t offset, FcitxCompareClosureFunc compare, void* data)
{
    FcitxListHead* result = NULL;
    if (!heada) {
        *end = endb;
        return headb;
    } else if (!headb) {
        *end = enda;
        return heada;
    }
    void* dataa =(void*) (((char*) heada) - offset);
    void* datab =(void*) (((char*) headb) - offset);
    if (compare(dataa, datab, data) <= 0) {
        result = heada;
        result->next = fcitx_list_sorted_merge(heada->next, enda, headb, endb, end, offset, compare, data);
    } else {
        result = headb;
        result->next = fcitx_list_sorted_merge(heada, enda, headb->next, endb, end, offset, compare, data);
    }
    if (result->next) {
        result->next->prev = result;
    } else {
        *end = result;
    }
    return result;
}

FCITX_EXPORT_API
void fcitx_list_sort(FcitxListHead* list, size_t offset, FcitxCompareFunc compare)
{
    fcitx_list_sort_r(list, offset, fcitx_list_cmp_helper, compare);
}

FCITX_EXPORT_API
void fcitx_list_sort_r(FcitxListHead* list, size_t offset, FcitxCompareClosureFunc compare, void* data)
{
    if (fcitx_list_is_empty(list)) {
        return;
    }
    
    if (list->prev == list->next) {
        return;
    }
        
    FcitxListHead* slow = list->next;
    FcitxListHead* fast = list->next->next;
    
    FcitxListHead* heada, *enda, *headb, *endb;
    while (fast != list) {
        fast = fast->next;
        if (fast != list) {
            slow = slow->next;
            fast = fast->next;
        }
    }
    
    headb = slow->next;
    endb = list->prev;
    
    // list->next = head; head->prev = slow;
    list->prev = slow;
    slow->next = list;
    fcitx_list_sort_r(list, offset, compare, data);
    heada = list->next;
    heada->prev = NULL;
    enda = list->prev;
    enda->next = NULL;
    
    list->next = headb;
    headb->prev = list;
    list->prev = endb;
    endb->next = list;
    fcitx_list_sort_r(list, offset, compare, data);
    
    headb = list->next;
    headb->prev = NULL;
    endb = list->prev;
    endb->next = NULL;
    
    FcitxListHead* end = NULL;
    FcitxListHead* head = fcitx_list_sorted_merge(heada, enda, headb, endb, &end, offset, compare, data);
    list->next = head;
    head->prev = list;
    list->prev = end;
    end->next = list;
}