/*
练习步骤见 Doc/01_链表.md。对照实现见 linked_list_ref.c。
同一时间只打开一个 SECTION。函数体留空，main 里的演示已经写好：
把函数填完后，输出应能对上 ref 里对应 SECTION。
编译：
gcc -std=c11 -Wall -Wextra -g -fsanitize=address -o linked_list linked_list.c
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>


#define SECTION_1 0
#define SECTION_2 1
#define SECTION_3 1



//单向链表
#if SECTION_1

typedef struct Node {
    int data;
    struct Node *next;
} Node;

static Node *node_new(int data)
{
    Node *n = malloc(sizeof(*n));
    if (n == NULL){
        return NULL;
    }
    n->data = data;
    n->next = NULL;
    return n;
}

static void push_front(Node **head, int data)
{
    Node *n = node_new(data);
    if (n == NULL){
        return;
    }
    n->next = *head;
    *head = n;
}

static void push_back(Node **head, int data)
{
    Node *n = node_new(data);
    if (n == NULL){
        return;
    }

    Node **pp = head;
    while (*pp != NULL){
        pp = &(*pp)->next;
    }

    *pp = n;
}

static void print_list(const char *tag, const Node *head)
{
    printf("%s", tag);
    const Node *p = head;
    while (p != NULL){
        printf("%d -> ", p->data);
        p = p->next;
    }
    printf("NULL\n");
}

static void reverse(Node **head)
{
    Node *prev = NULL;
    Node *Curr = *head;
    while (Curr != NULL){
        Node *next = Curr->next;
        Curr->next = prev;
        prev = Curr;
        Curr = next;
    }
    *head = prev;
}

static Node *find_value(Node *head, int data)
{
    Node *p = head;
    while (p != NULL){
        if (p->data == data){
            return p;
        }
        p = p->next;
    }
    return NULL;
}

static void erase(Node **head, Node *target)
{
    if (head == NULL){
        return;
    }
    Node **pp = head;
    while (*pp != NULL && *pp != target){
        pp = &(*pp)->next;
    }
    if (*pp == NULL){
        return;
    }
    *pp = target->next;
    free(target);
}

static void destroy(Node **head)
{
    Node *p = *head;
    while (p != NULL){
        Node *next = p->next;
        free(p);
        p = next;
    }
    *head = NULL;
}

#endif


//双向循环链表
#if SECTION_2

typedef struct DNode {
    int data;
    struct DNode *prev;
    struct DNode *next;
} DNode;

static void dlist_init(DNode *sentinel)
{
    (void)sentinel;
}

static int dlist_is_empty(const DNode *sentinel)
{
    (void)sentinel;
    return 1;
}

static DNode *dnode_new(int data)
{
    (void)data;
    return NULL;
}

static void dlist_insert_after(DNode *pos, DNode *node)
{
    (void)pos;
    (void)node;
}

static void dlist_insert_tail(DNode *sentinel, DNode *node)
{
    (void)sentinel;
    (void)node;
}

static void dlist_erase(DNode *node)
{
    (void)node;
}

static void dlist_splice_after(DNode *pos, DNode *first, DNode *last)
{
    (void)pos;
    (void)first;
    (void)last;
}

static void dlist_print(const char *tag, const DNode *sentinel)
{
    (void)tag;
    (void)sentinel;
}

static void dlist_destroy_nodes(DNode *sentinel)
{
    (void)sentinel;
}

#endif




// 侵入式链表
#if SECTION_3

struct list_head {
    struct list_head *next;
    struct list_head *prev;
};

struct Job {
    int cmd;
    int arg;
    struct list_head link;
};

#define JOB_POOL_LEN 8

static struct Job jobs[JOB_POOL_LEN];
static struct list_head free_list;
static struct list_head work_list;

#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

static void list_init(struct list_head *head)
{
    (void)head;
}

static int list_is_empty(const struct list_head *head)
{
    (void)head;
    return 1;
}

static void list_insert_after(struct list_head *pos, struct list_head *node)
{
    (void)pos;
    (void)node;
}

static void list_insert_tail(struct list_head *head, struct list_head *node)
{
    (void)head;
    (void)node;
}

static void list_erase(struct list_head *node)
{
    (void)node;
}

static struct Job *job_from_link(struct list_head *link)
{
    (void)link;
    return NULL;
}

static void pool_init(void)
{
}

static struct Job *job_alloc(void)
{
    return NULL;
}

static void job_free(struct Job *job)
{
    (void)job;
}

static struct Job *submit(int cmd, int arg)
{
    (void)cmd;
    (void)arg;
    return NULL;
}

static int run_one(void)
{
    return 0;
}

static void cancel(struct Job *job)
{
    (void)job;
}

static void print_chain(const char *tag, struct list_head *head)
{
    (void)tag;
    (void)head;
}

#endif


int main(void)
{
#if SECTION_1
    Node *head = NULL;

    push_back(&head, 10);
    push_back(&head, 20);
    push_back(&head, 30);
    push_front(&head, 1);
    print_list("build:   ", head);

    reverse(&head);
    print_list("reverse: ", head);

    erase(&head, find_value(head, 1));
    print_list("erase 1: ", head);

    erase(&head, find_value(head, 20));
    print_list("erase20: ", head);

    destroy(&head);
    print_list("destroy: ", head);
#endif

#if SECTION_2
    DNode a;
    DNode b;
    DNode *n1;
    DNode *n2;
    DNode *n3;
    DNode *n4;

    dlist_init(&a);
    dlist_init(&b);

    n1 = dnode_new(1);
    n2 = dnode_new(2);
    n3 = dnode_new(3);
    n4 = dnode_new(4);
    dlist_insert_tail(&a, n1);
    dlist_insert_tail(&a, n2);
    dlist_insert_tail(&a, n3);
    dlist_insert_tail(&a, n4);
    dlist_print("A build:     ", &a);
    printf("B empty? %d\n", dlist_is_empty(&b));
    dlist_print("B empty:     ", &b);

    dlist_splice_after(&b, n2, n3);
    dlist_print("A after cut: ", &a);
    dlist_print("B after add: ", &b);

    dlist_erase(n1);
    free(n1);
    dlist_print("A erase 1:   ", &a);

    dlist_destroy_nodes(&a);
    dlist_destroy_nodes(&b);
#endif

#if SECTION_3
    struct Job *kept;
    int i;

    pool_init();
    print_chain("free  init: ", &free_list);
    print_chain("work  init: ", &work_list);

    submit(1, 10);
    submit(2, 20);
    kept = submit(3, 30);
    print_chain("work  3job: ", &work_list);

    run_one();
    run_one();
    print_chain("work  left: ", &work_list);

    cancel(kept);
    print_chain("work  none: ", &work_list);

    for (i = 0; i < JOB_POOL_LEN + 1; i++) {
        struct Job *job = submit(100 + i, i);
        if (job == NULL) {
            printf("pool exhausted at submit #%d\n", i);
            break;
        }
    }
    print_chain("work  full: ", &work_list);

    while (run_one()) {
    }
    print_chain("free  back: ", &free_list);
#endif

    return 0;
}
