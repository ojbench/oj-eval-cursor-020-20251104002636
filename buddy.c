#include "buddy.h"
#define NULL ((void *)0)

#define MAXRANK 16
#define PAGE_SIZE (4 * 1024)

// Data structures for buddy allocator
typedef struct free_block {
    struct free_block *next;
} free_block_t;

// Free lists for each rank
static free_block_t *free_lists[MAXRANK + 1];

// Memory pool information
static void *pool_start;
static int total_pages;

// Page metadata - stores rank for each page
// Positive value = allocated with that rank
// Negative value = free with that rank
// 0 = part of a larger block
#define MAX_PAGES (256 * 1024)  // Support up to 256K pages (1GB memory)
static char page_rank[MAX_PAGES];

// Helper function to get page index from address
static inline int get_page_index(void *p) {
    return ((char *)p - (char *)pool_start) / PAGE_SIZE;
}

// Helper function to get address from page index
static inline void *get_page_address(int idx) {
    return (char *)pool_start + idx * PAGE_SIZE;
}

// Helper function to get buddy index
static inline int get_buddy_index(int idx, int rank) {
    int block_size = (1 << (rank - 1));
    return idx ^ block_size;
}

// Check if address is valid
static inline int is_valid_address(void *p) {
    if (p < pool_start) return 0;
    int idx = get_page_index(p);
    if (idx >= total_pages) return 0;
    if (get_page_address(idx) != p) return 0;  // Not aligned
    return 1;
}

int init_page(void *p, int pgcount) {
    pool_start = p;
    total_pages = pgcount;
    
    // Initialize free lists
    for (int i = 0; i <= MAXRANK; i++) {
        free_lists[i] = NULL;
    }
    
    // Initialize page ranks
    for (int i = 0; i < total_pages; i++) {
        page_rank[i] = 0;
    }
    
    // Add the entire pool to the appropriate free lists
    // We need to build the largest possible blocks
    int idx = 0;
    while (idx < total_pages) {
        int remaining = total_pages - idx;
        int rank;
        
        // Find the largest rank that fits
        for (rank = MAXRANK; rank >= 1; rank--) {
            int block_size = (1 << (rank - 1));
            if (block_size <= remaining && (idx % block_size) == 0) {
                break;
            }
        }
        
        if (rank < 1) break;
        
        int block_size = (1 << (rank - 1));
        void *block_addr = get_page_address(idx);
        
        // Add to free list
        free_block_t *block = (free_block_t *)block_addr;
        block->next = free_lists[rank];
        free_lists[rank] = block;
        
        // Mark pages
        page_rank[idx] = -rank;
        for (int i = 1; i < block_size; i++) {
            page_rank[idx + i] = 0;
        }
        
        idx += block_size;
    }
    
    return OK;
}

void *alloc_pages(int rank) {
    // Validate rank
    if (rank < 1 || rank > MAXRANK) {
        return ERR_PTR(-EINVAL);
    }
    
    // Find a free block of the requested rank or larger
    int found_rank = -1;
    for (int r = rank; r <= MAXRANK; r++) {
        if (free_lists[r] != NULL) {
            found_rank = r;
            break;
        }
    }
    
    if (found_rank == -1) {
        return ERR_PTR(-ENOSPC);
    }
    
    // Remove block from free list
    free_block_t *block = free_lists[found_rank];
    free_lists[found_rank] = block->next;
    
    void *addr = (void *)block;
    int idx = get_page_index(addr);
    
    // Split if necessary
    while (found_rank > rank) {
        found_rank--;
        int block_size = (1 << (found_rank - 1));
        
        // Split into two buddies
        void *buddy_addr = get_page_address(idx + block_size);
        free_block_t *buddy = (free_block_t *)buddy_addr;
        buddy->next = free_lists[found_rank];
        free_lists[found_rank] = buddy;
        
        // Mark buddy as free
        int buddy_idx = idx + block_size;
        page_rank[buddy_idx] = -found_rank;
        for (int i = 1; i < block_size; i++) {
            page_rank[buddy_idx + i] = 0;
        }
    }
    
    // Mark as allocated
    int block_size = (1 << (rank - 1));
    page_rank[idx] = rank;
    for (int i = 1; i < block_size; i++) {
        page_rank[idx + i] = 0;
    }
    
    return addr;
}

int return_pages(void *p) {
    // Validate address
    if (!is_valid_address(p)) {
        return -EINVAL;
    }
    
    int idx = get_page_index(p);
    int rank = page_rank[idx];
    
    // Check if it's allocated
    if (rank <= 0) {
        return -EINVAL;
    }
    
    int block_size = (1 << (rank - 1));
    
    // Try to merge with buddy
    while (rank < MAXRANK) {
        int buddy_idx = get_buddy_index(idx, rank);
        
        // Check if buddy exists and is free with same rank
        if (buddy_idx >= 0 && buddy_idx < total_pages) {
            if (page_rank[buddy_idx] == -rank) {
                // Buddy is free, merge them
                
                // Remove buddy from free list
                void *buddy_addr = get_page_address(buddy_idx);
                free_block_t **prev = &free_lists[rank];
                while (*prev != NULL) {
                    if ((void *)(*prev) == buddy_addr) {
                        *prev = (*prev)->next;
                        break;
                    }
                    prev = &((*prev)->next);
                }
                
                // Determine the start of merged block
                if (buddy_idx < idx) {
                    idx = buddy_idx;
                }
                
                rank++;
                block_size = (1 << (rank - 1));
                continue;
            }
        }
        break;
    }
    
    // Add merged block to free list
    void *addr = get_page_address(idx);
    free_block_t *block = (free_block_t *)addr;
    block->next = free_lists[rank];
    free_lists[rank] = block;
    
    // Mark as free
    page_rank[idx] = -rank;
    for (int i = 1; i < block_size; i++) {
        page_rank[idx + i] = 0;
    }
    
    return OK;
}

int query_ranks(void *p) {
    // Validate address
    if (!is_valid_address(p)) {
        return -EINVAL;
    }
    
    int idx = get_page_index(p);
    int rank = page_rank[idx];
    
    if (rank > 0) {
        return rank;  // Allocated
    } else if (rank < 0) {
        return -rank;  // Free
    } else {
        return -EINVAL;  // Part of larger block
    }
}

int query_page_counts(int rank) {
    // Validate rank
    if (rank < 1 || rank > MAXRANK) {
        return -EINVAL;
    }
    
    // Count blocks in free list
    int count = 0;
    free_block_t *block = free_lists[rank];
    while (block != NULL) {
        count++;
        block = block->next;
    }
    
    return count;
}
