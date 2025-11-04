# Buddy Algorithm Implementation Summary

## Problem
Implement a buddy memory allocation algorithm for managing 4KB pages with ranks 1-16.

## Attempts Made (3/3 submissions)

### Submission 1: Initial Implementation
- **Commit**: 31e5a63
- **Approach**: Basic buddy algorithm with singly-linked free lists
- **Result**: Time Limit Exceeded (13009ms / 10000ms limit)
- **Issue**: Linear search O(n) when removing buddy from free list during merging

### Submission 2: Doubly-Linked List Optimization
- **Commit**: 4de2a1d
- **Approach**: Converted free lists to doubly-linked lists for O(1) removal
- **Result**: Time Limit Exceeded (13008ms / 10000ms limit)
- **Issue**: Still had loops marking interior pages

### Submission 3: Remove Interior Page Marking
- **Commit**: 25f6035
- **Approach**: Only mark first page of each block, eliminate O(block_size) loops
- **Result**: Time Limit Exceeded (13008ms / 10000ms limit)
- **Issue**: Unknown - consistent 13008ms suggests systematic bottleneck

## Implementation Details

### Data Structures
- Free lists: Doubly-linked lists for each rank (1-16)
- Page metadata: `page_rank[MAX_PAGES]` array tracking block ranks
  - Positive: allocated block of that rank
  - Negative: free block of that rank
  - Zero: part of larger block

### Key Optimizations Applied
1. Doubly-linked lists for O(1) removal
2. Eliminated interior page marking loops
3. Inline helper functions
4. Direct array lookups instead of searches

### Algorithm Complexity
- `alloc_pages`: O(log MAXRANK) for splitting
- `return_pages`: O(log MAXRANK) for merging with O(1) buddy removal
- `query_ranks`: O(1)
- `query_page_counts`: O(count) for counting free blocks

## Observations
- Local tests pass successfully
- Memory usage: ~135 MB (expected for 32K pages * 4KB)
- Time is consistently 13008ms across all submissions
- All functional tests pass locally

## Possible Remaining Issues
1. OJ test cases may be larger or more complex than local tests
2. Initialization overhead with MAX_PAGES array (256KB)
3. Memory access patterns causing cache misses
4. OJ environment may be significantly slower than local
5. There may be additional test workloads not in main.c

## Final Score: 0 / 100
All 3 submissions resulted in Time Limit Exceeded.
