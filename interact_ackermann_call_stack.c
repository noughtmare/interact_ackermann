#include <sys/mman.h>

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

// cells can only have two non-primary ports
struct Cell {
    intptr_t id; // the type (or identity) of a cell
    intptr_t p1; // the first non-primary port
    intptr_t p2; // the second non-primary port
};

void apply_rule(struct Cell*, struct Cell*);

/*/ BEGIN CUSTOM ALLOCATOR

struct Cell *mem;
struct Cell *head = 0;

void ia_initialize() {
    mem = mmap(0, (long) 1024 * 1024 * 1024 * 1024, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
}

static inline struct Cell __attribute__ ((always_inline, malloc)) *ia_alloc() {
    struct Cell *result;
    
    if (head != 0) {
        result = head;
        head = (struct Cell *)head->id;
    } else {
        result = mem;
        mem += sizeof(struct Cell);
    }
    
    return result;
}

static inline void __attribute__ ((always_inline)) ia_free(struct Cell* x) {
    x->id = (intptr_t) head;
    head = x;
}

// END CUSTOM ALLOCATOR /*/

// to increase stack size
#include <sys/resource.h>

// fast block allocator
#include "gjalloc.h"


// debug 0 = no debugging info
// debug 1 = show diagnostics
// debug 2 = show which rules get fired
#define DEBUG 0

// safe checks if tagged pointers are possible
// #define SAFE

// number of interaction rules
#define NROF_RULES 6

// Cell types
// : Nat+
#define ZERO  0
// : Nat+; Nat-
#define SUCC  1
// : Nat-; Nat+, Nat+
#define DUP   2
// : Nat-
#define ERASE 3
// : Nat-; Nat-, Nat+
#define A     4
// : Nat-; Nat-, Nat+
#define AA    5

/*/ BEGIN STACK

struct Cell **stack_base;
struct Cell **stack_head;

inline void __attribute__ ((always_inline)) push(struct Cell* val) {
    *(stack_head++) = val;
}

inline struct Cell * __attribute__ ((always_inline)) pop() {
    return *--stack_head;
}

// END STACK */

struct block_allocator allocator = BA_INIT(sizeof(struct Cell),16384);

#if DEBUG > 0
long long nr_of_interactions = 0;
long long max_depth = 0;
long long cur_depth = 0;
long long counts[] = {0,0,0,0,0,0,0,0,0,0,0};
int count_index = 0;
#endif

const intptr_t ptr_mask = ~1;

// create a new cell of type id
static inline struct Cell* __attribute__((always_inline)) new_cell(intptr_t id) {
    //struct Cell* result = ia_alloc();
    struct Cell* result = ba_alloc(&allocator);
    //struct Cell* result = (struct Cell*) malloc(sizeof(struct Cell));
#ifdef SAFE
    if ((intptr_t) result & 1) {
        fprintf(stderr, "Pointers not aligned! Exiting...\n");
        exit(-1);
    }
#endif
    result->id = id;
    return result;
}

static inline struct Cell* __attribute__((always_inline)) free_cell(struct Cell* cell) {
    //ia_free(cell);
    ba_free(&allocator, cell);
    //free(cell);
}

inline struct Cell* __attribute__((always_inline)) repurpose(struct Cell* cell, intptr_t id) {
    cell->id = id;
    return cell;
}

inline void __attribute__((always_inline)) connect_one_primary(intptr_t x, struct Cell* cell) {
    if (!(x & 1)) {
        *((intptr_t*) x) = (intptr_t) cell | 1;
    } else {
#if DEBUG > 0
        counts[count_index + 0]++;
#endif
        apply_rule((struct Cell *) (x & ptr_mask), cell);
    }
}

inline void __attribute__((always_inline)) connect_two_primary(intptr_t x, struct Cell* cell1, intptr_t y, struct Cell* cell2) {
    if (!(x & 1)) {
        *((intptr_t*) x) = (intptr_t) cell1 | 1;
    }
    connect_one_primary(y,cell2);
    if (x & 1) {
#if DEBUG > 0
        counts[count_index + 1]++;
#endif
        apply_rule((struct Cell *) (x & ptr_mask), cell1);
    }
}

inline void __attribute__((always_inline)) connect_three_primary(intptr_t x, struct Cell* cell1, intptr_t y, struct Cell* cell2, intptr_t z, struct Cell* cell3) {
    if (!(x & 1)) {
        *((intptr_t*) x) = (intptr_t) cell1 | 1;
    }
    connect_two_primary(y,cell2,z,cell3);
    if (x & 1) {
#if DEBUG > 0
        counts[count_index + 2]++;
#endif
        apply_rule((struct Cell*) (x & ptr_mask), cell1);
    }
}

// inline void __attribute__((always_inline)) connect_four_primary(intptr_t x, struct Cell* cell1, intptr_t y, struct Cell* cell2, intptr_t z, struct Cell* cell3, intptr_t w, struct Cell* cell4) {
//     if (!(x & 1)) {
//         *((intptr_t*) x) = (intptr_t) cell1 | 1;
//     }
//     connect_three_primary(y,cell2,z,cell3,w,cell4);
//     if (x & 1) {
// #if DEBUG > 0
//         counts[count_index + 3]++;
// #endif
//         push((struct Cell*) (x & ptr_mask));
//         push(cell1);
//         // apply_rule();
//     }
// }

inline void __attribute__((always_inline)) connect_p1_p1(struct Cell* cell1, struct Cell* cell2) {
    cell1->p1 = (intptr_t) &(cell2->p1);
    cell2->p1 = (intptr_t) &(cell1->p1);
}
inline void __attribute__((always_inline)) connect_p1_p2(struct Cell* cell1, struct Cell* cell2) {
    cell1->p1 = (intptr_t) &(cell2->p2);
    cell2->p2 = (intptr_t) &(cell1->p1);
}
inline void __attribute__((always_inline)) connect_p2_p2(struct Cell* cell1, struct Cell* cell2) {
    cell1->p2 = (intptr_t) &(cell2->p2);
    cell2->p2 = (intptr_t) &(cell1->p2);
}

inline void __attribute__((always_inline)) connect_secondary_p1(intptr_t x, struct Cell* cell) {
    cell->p1 = x;
    if (!(x & 1)) {
        *((intptr_t*) x) = (intptr_t) &(cell->p1);
    }
}

void rule_0_2(struct Cell* zero1, struct Cell* dup) {
    intptr_t x = dup->p1;
    intptr_t y = dup->p2;

    struct Cell* zero2 = repurpose(dup, ZERO);

#if DEBUG > 0
    count_index = 0;
#endif
    connect_two_primary(y, zero2, x, zero1);
}

void rule_1_2(struct Cell* succ1, struct Cell* dup) {
    struct Cell* succ2 = new_cell(SUCC);

    intptr_t x = dup->p1;
    intptr_t y = dup->p2;
    intptr_t z = succ1->p1;

    connect_p1_p1(succ1,dup);
    connect_p1_p2(succ2,dup);
#if DEBUG > 0
    count_index = 2;
#endif
    connect_three_primary(y, succ2, x, succ1, z, dup);
}

long erase_count = 0;

void rule_0_3(struct Cell* zero, struct Cell* erase) {
    printf("%ld\n", erase_count);

    free_cell(zero);
    free_cell(erase);
}

void rule_1_3(struct Cell* succ, struct Cell* erase) {
    erase_count++;

    intptr_t x = succ->p1;
    free_cell(succ);

#if DEBUG > 0
    count_index = 5;
#endif
    connect_one_primary(x,erase);
}

void rule_0_4(struct Cell* zero, struct Cell* a) {
    free_cell(zero);

    struct Cell* succ = repurpose(a, SUCC);

#if DEBUG > 0
    count_index = 6;
#endif
    connect_one_primary(a->p2, succ);
}

void rule_1_4(struct Cell* succ, struct Cell* a) {
    intptr_t m = succ->p1;
    free_cell(succ);

    intptr_t n = a->p1;

    struct Cell* aa = repurpose(a, AA);

    connect_secondary_p1(m, aa);
#if DEBUG > 0
    count_index = 7;
#endif
    connect_one_primary(n, aa);
}

void rule_0_5(struct Cell* zero, struct Cell* aa) {
    struct Cell* succ = new_cell(SUCC);
    succ->p1 = (intptr_t) zero | 1;

    intptr_t m = aa->p1;

    struct Cell* a = repurpose(aa, A);
    a->p1 = (intptr_t) succ | 1;

#if DEBUG > 0
    count_index = 8;
#endif
    connect_one_primary(m, a);
}

void rule_1_5(struct Cell* succ, struct Cell* aa) {
    struct Cell* dup = new_cell(DUP);
    struct Cell* a = repurpose(aa, A);
    struct Cell* aaa = repurpose(succ, AA);

    intptr_t n = succ->p1;
    intptr_t m = aa->p1;

    dup->p1 = (intptr_t) a | 1;

    connect_p1_p2(aaa,dup);
    connect_p1_p2(a,aaa);
#if DEBUG > 0
    count_index = 9;
#endif
    connect_two_primary(m, dup, n, aaa);
}

void lookup_rule(int aid, int bid, struct Cell* a, struct Cell* b) {
#if DEBUG > 1
    printf("looking up rule %d %d...\n", aid, bid);
#endif
    switch(bid + aid * NROF_RULES) {
        case  2: rule_0_2(a,b); break;
        case  3: rule_0_3(a,b); break;
        case  4: rule_0_4(a,b); break;
        case  5: rule_0_5(a,b); break;
        case  8: rule_1_2(a,b); break;
        case  9: rule_1_3(a,b); break;
        case 10: rule_1_4(a,b); break;
        case 11: rule_1_5(a,b); break;
        default: break;
    }
}

void apply_rule(struct Cell* a, struct Cell* b) {
#if DEBUG > 0
    nr_of_interactions++;
    cur_depth++;
    if (cur_depth > max_depth) {
        max_depth = cur_depth;
    }
#endif

    int aid = a->id;
    int bid = b->id;

    if (aid > bid) {
        lookup_rule(bid,aid,b,a);
    } else {
        lookup_rule(aid,bid,a,b);
    }
#if DEBUG > 0
    cur_depth--;
#endif
}

int main() {
    //ia_initialize();
    
    // stack_base = malloc(1 * sizeof(struct Cell));
    //stack_base = malloc(100000 * sizeof(struct Cell));
    // stack_base = mmap(0, (long) 1024 * 1024 * 1024 * 1024, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
    // stack_head = stack_base;

#if DEBUG > 2
    printf("initialised\n");
#endif

    if ((sizeof(intptr_t) & 1)) {
        fprintf(stderr, "intptr_t has odd size! Exiting...\n");
        exit(-1);
    }
    struct Cell* a     = new_cell(A);
    struct Cell* s1    = new_cell(SUCC);
    struct Cell* s2    = new_cell(SUCC);
    struct Cell* s3    = new_cell(SUCC);
    struct Cell* s4    = new_cell(SUCC);
    struct Cell* s5    = new_cell(SUCC);
    struct Cell* s6    = new_cell(SUCC);
    struct Cell* s7    = new_cell(SUCC);
    struct Cell* s8    = new_cell(SUCC);
    struct Cell* s9    = new_cell(SUCC);
    struct Cell* s10   = new_cell(SUCC);
    struct Cell* s11   = new_cell(SUCC);
    struct Cell* s12   = new_cell(SUCC);
    struct Cell* s13   = new_cell(SUCC);
    struct Cell* s14   = new_cell(SUCC);
    struct Cell* s15   = new_cell(SUCC);
    struct Cell* s16   = new_cell(SUCC);
    struct Cell* s17   = new_cell(SUCC);
    struct Cell* s18   = new_cell(SUCC);
    struct Cell* s19   = new_cell(SUCC);
    struct Cell* z1    = new_cell(ZERO);
    struct Cell* z2    = new_cell(ZERO);
    struct Cell* erase = new_cell(ERASE);

    a->p1   = (intptr_t) s1 | 1;
    a->p2   = (intptr_t) erase | 1;
    s1->p1  = (intptr_t) s6 | 1;
    s2->p1  = (intptr_t) s3 | 1;
    s3->p1  = (intptr_t) s4 | 1;
    s4->p1  = (intptr_t) z2 | 1;
    s5->p1  = (intptr_t) z2 | 1;
    s6->p1  = (intptr_t) s7 | 1;
    s7->p1  = (intptr_t) s8 | 1;
    s8->p1  = (intptr_t) s9 | 1;
    s9->p1  = (intptr_t) s10 | 1;
    s10->p1 = (intptr_t) s11 | 1;
    s11->p1 = (intptr_t) s12 | 1;
    s12->p1 = (intptr_t) s13 | 1;
    s13->p1 = (intptr_t) s14 | 1;
    s14->p1 = (intptr_t) s15 | 1;
    s15->p1 = (intptr_t) s16 | 1;
    s16->p1 = (intptr_t) z1 | 1;
    s17->p1 = (intptr_t) z1 | 1;
    s18->p1 = (intptr_t) z1 | 1;
    s19->p1 = (intptr_t) z1 | 1;

#if DEBUG > 2
    printf("built\n");
#endif

    // push(a);
    // push(s2);

    apply_rule(a, s2);

//     while (stack_head > stack_base) {
//         apply_rule();
//     }

#if DEBUG > 0
    printf("%lld interactions, %lld maximum depth\n", nr_of_interactions, max_depth);
    for(int i = 0; i < 11; i++) {
        printf("%d:\t%lld\n", i, counts[i]);
    }
#endif

    return 0;
}
