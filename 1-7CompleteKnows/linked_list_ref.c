/*
对照实现，不是练习答案纸。先编译跑通，对着 Doc/01_链表.md 画箭头，
再关掉本文件，去 linked_list.c 里默写。
编译：
gcc -std=c11 -Wall -Wextra -g -fsanitize=address -o linked_list_ref linked_list_ref.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

/* -------------------------------------------------------------------------- */
/* 第 1 步：单链表                                                              */
/* -------------------------------------------------------------------------- */

typedef struct Node {
    int data;
    struct Node *next;
} Node;

static Node *node_new(int data)
{
    Node *n = malloc(sizeof(*n));   //开辟了多少空间？n不是一个指针吗？,开辟的就是一个节点大小的空间
    if (n == NULL) {
        return NULL;
    }
    n->data = data;
    n->next = NULL;
    return n;
}

/* 新节点成为新的头，原头变成它的后继 */
static void push_front(Node **head, int data)
{
    Node *n = node_new(data);
    if (n == NULL) {
        return;
    }
    n->next = *head;
    *head = n;
}

/* 走到「指向空尾巴的那根指针」，把新节点接上去；空表时 pp 就是 head 本身 */
static void push_back(Node **head, int data)
{
    Node *n = node_new(data);
    if (n == NULL) {
        return;
    }

    Node **pp = head;
    while (*pp != NULL) {
        pp = &(*pp)->next;
    }
    *pp = n;
}

static void print_list(const char *tag, const Node *head)
{
    printf("%s", tag);
    const Node *p = head;
    while (p != NULL) {
        printf("%d -> ", p->data);
        p = p->next;
    }
    printf("NULL\n");
}

/*
 * 反转只改 next 的方向，节点本身不搬家。
 * 循环里必须先保住后继，再把当前的 next 拧向前驱，否则链会丢。
 */
static void reverse(Node **head)
{
    Node *prev = NULL;
    Node *curr = *head;

    while (curr != NULL) {
        Node *next = curr->next;
        curr->next = prev;
        prev = curr;
        curr = next;
    }
    *head = prev;
}

/* 按值找到第一个匹配节点，找不到返回 NULL。给 erase 提供「已有句柄」 */
static Node *find_value(Node *head, int data)
{
    while (head != NULL && head->data != data) {
        head = head->next;
    }
    return head;
}

/*
 * pp 始终指向「指向当前节点的那根指针」。
 * 找到 target 后，把这根指针改接到 target 的后继，头节点与中间节点同一套代码。
 */
static void erase(Node **head, Node *target)
{
    if (target == NULL) {
        return;
    }

    Node **pp = head;
    while (*pp != NULL && *pp != target) {
        pp = &(*pp)->next;
    }
    if (*pp == NULL) {
        return;
    }

    *pp = target->next;
    free(target);
}

static void destroy(Node **head)
{
    Node *p = *head;
    while (p != NULL) {
        Node *next = p->next;
        free(p);
        p = next;
    }
    *head = NULL;
}

static void demo_singly(void)
{
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
}

/* -------------------------------------------------------------------------- */
/* 第 2 步：双向循环 + 哨兵                                                     */
/* -------------------------------------------------------------------------- */

typedef struct DNode {
    int data;
    struct DNode *prev;
    struct DNode *next;
} DNode;

/* 哨兵自环表示空表：next / prev 都指向自己 */
static void dlist_init(DNode *sentinel)
{
    sentinel->prev = sentinel;
    sentinel->next = sentinel;
    sentinel->data = 0;
}

static int dlist_is_empty(const DNode *sentinel)
{
    return sentinel->next == sentinel;
}

static DNode *dnode_new(int data)
{
    DNode *n = malloc(sizeof(*n));
    if (n == NULL) {
        return NULL;
    }
    n->data = data;
    n->prev = NULL;
    n->next = NULL;
    return n;
}

/*
 * 把 node 插到 pos 后面。
 * 先抓住原后继，再把 node 的两头、pos 的 next、原后继的 prev 全部接好。
 */
static void dlist_insert_after(DNode *pos, DNode *node)
{
    DNode *succ = pos->next;

    node->prev = pos;
    node->next = succ;
    pos->next = node;
    succ->prev = node;
}

/* 等价于插到哨兵前面，也就是当前尾部之后 */
static void dlist_insert_tail(DNode *sentinel, DNode *node)
{
    dlist_insert_after(sentinel->prev, node);
}

/*
 * 已知节点删除：只改前驱与后继的互指，不遍历。
 * 调用方保证 node 不是哨兵。
 */
static void dlist_erase(DNode *node)
{
    DNode *pred = node->prev;
    DNode *succ = node->next;

    pred->next = succ;
    succ->prev = pred;
    node->prev = NULL;
    node->next = NULL;
}

/*
 * 把闭区间 [first, last] 从原位置剪下，接到 pos 后面。
 * 节点不释放、不拷贝。pos 不得落在 [first, last] 内。
 */
static void dlist_splice_after(DNode *pos, DNode *first, DNode *last)
{
    DNode *old_pred = first->prev;
    DNode *old_succ = last->next;
    DNode *new_succ = pos->next;

    old_pred->next = old_succ;
    old_succ->prev = old_pred;

    pos->next = first;
    first->prev = pos;
    last->next = new_succ;
    new_succ->prev = last;
}

static void dlist_print(const char *tag, const DNode *sentinel)
{
    printf("%s", tag);
    const DNode *p = sentinel->next;
    while (p != sentinel) {
        printf("%d <-> ", p->data);
        p = p->next;
    }
    printf("(sentinel)\n");
}

static void dlist_destroy_nodes(DNode *sentinel)
{
    DNode *p = sentinel->next;
    while (p != sentinel) {
        DNode *next = p->next;
        free(p);
        p = next;
    }
    dlist_init(sentinel);
}

static void demo_dlist(void)
{
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

    /* 把 A 上的 [2, 3] 整段挪到 B */
    dlist_splice_after(&b, n2, n3);
    dlist_print("A after cut: ", &a);
    dlist_print("B after add: ", &b);

    dlist_erase(n1);
    free(n1);
    dlist_print("A erase 1:   ", &a);

    dlist_destroy_nodes(&a);
    dlist_destroy_nodes(&b);
}

/* -------------------------------------------------------------------------- */
/* 第 3 步：侵入式链表 + 对象池                                                 */
/* -------------------------------------------------------------------------- */

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
    head->next = head;
    head->prev = head;
}

static int list_is_empty(const struct list_head *head)
{
    return head->next == head;
}

static void list_insert_after(struct list_head *pos, struct list_head *node)
{
    struct list_head *succ = pos->next;

    node->prev = pos;
    node->next = succ;
    pos->next = node;
    succ->prev = node;
}

static void list_insert_tail(struct list_head *head, struct list_head *node)
{
    list_insert_after(head->prev, node);
}

static void list_erase(struct list_head *node)
{
    struct list_head *pred = node->prev;
    struct list_head *succ = node->next;

    pred->next = succ;
    succ->prev = pred;
    node->next = node;
    node->prev = node;
}

static struct Job *job_from_link(struct list_head *link)
{
    return container_of(link, struct Job, link);
}

static void pool_init(void)
{
    size_t i;

    list_init(&free_list);
    list_init(&work_list);

    for (i = 0; i < JOB_POOL_LEN; i++) {
        jobs[i].cmd = 0;
        jobs[i].arg = 0;
        list_init(&jobs[i].link);
        list_insert_tail(&free_list, &jobs[i].link);
    }
}

static struct Job *job_alloc(void)
{
    struct list_head *link;

    if (list_is_empty(&free_list)) {
        return NULL;
    }
    link = free_list.next;
    list_erase(link);
    return job_from_link(link);
}

static void job_free(struct Job *job)
{
    job->cmd = 0;
    job->arg = 0;
    list_erase(&job->link);
    list_insert_after(&free_list, &job->link);
}

static struct Job *submit(int cmd, int arg)
{
    struct Job *job = job_alloc();
    if (job == NULL) {
        return NULL;
    }
    job->cmd = cmd;
    job->arg = arg;
    list_insert_tail(&work_list, &job->link);
    return job;
}

static int run_one(void)
{
    struct list_head *link;
    struct Job *job;

    if (list_is_empty(&work_list)) {
        return 0;
    }
    link = work_list.next;
    job = job_from_link(link);
    printf("run cmd=%d arg=%d\n", job->cmd, job->arg);
    job_free(job);
    return 1;
}

static void cancel(struct Job *job)
{
    if (job == NULL) {
        return;
    }
    printf("cancel cmd=%d arg=%d\n", job->cmd, job->arg);
    job_free(job);
}

static void print_chain(const char *tag, struct list_head *head)
{
    struct list_head *p;

    printf("%s", tag);
    for (p = head->next; p != head; p = p->next) {
        struct Job *job = job_from_link(p);
        printf("[cmd=%d arg=%d] ", job->cmd, job->arg);
    }
    printf("\n");
}

static void demo_pool(void)
{
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
}

int main(void)
{
    printf("===== SECTION 1 =====\n");
    demo_singly();
    printf("===== SECTION 2 =====\n");
    demo_dlist();
    printf("===== SECTION 3 =====\n");
    demo_pool();
    return 0;
}
