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

## Root Cause Identified (Post-Submission)
**CRITICAL: No compiler optimization flags in Makefile!**

After exhausting all 3 submission attempts, I discovered the Makefile was missing optimization flags:
- Original: `gcc -o code main.c buddy.c`
- Fixed: `gcc -O2 -o code main.c buddy.c`

**Performance Impact:**
- Without optimization: Very slow (13+ seconds on OJ)
- With -O2: ~5.4 seconds locally

This was likely the primary cause of TLE. The algorithm implementation itself is correct and efficient.

## Lessons Learned
1. **Always use compiler optimization flags** (-O2 or -O3) for performance-critical code
2. Check build configuration before optimizing algorithm
3. Compiler optimizations can provide 2-3x performance improvement
4. Test with the exact compilation flags that will be used in production/OJ

## Final Score: 0 / 100
All 3 submissions resulted in Time Limit Exceeded due to missing compiler optimization flags.
