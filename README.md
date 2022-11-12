# use after free(): how to exploit a flaw in glibc's heap management.

### what is heap? Please read about [heap vs. stack](https://www.geeksforgeeks.org/stack-vs-heap-memory-allocation/) if unfamiliar. 
your computer has storage (in this case, random access memory) which is always available for use by C executables. 
any given program starts at a location in that 'storage', and the stack specific to that program grows from that location onwards. 
the stack is fast to access, but it's only around 8 MB of space. in comparison, the heap is for dynamically allocated memory,
which can be much larger than 8 megabytes. as a downside, it's slightly slower to access (and you have to free the allocated memory manually)

### dynamically allocated memory: memory allocated during runtime. 

`malloc(size_t size)` Allocates size bytes and returns that pointer to the programmer

`calloc(size_t, nmemb, size_t size)` Allocates nmemb*size bytes and zeros out the memory.

`free(void *ptr)` Frees the heap space pointed by ptr

what you need to know: dynamically allocating memory stores the 'information' in the heap instead of the stack (pictured below). to free this information stored in the heap, you use the `free()` function.

![IMG_0274](https://user-images.githubusercontent.com/114739901/201461944-9b8fb959-10ff-44f5-ac70-4b77dca65a17.jpg)

### what does it mean to `free()` allocated memory?
haha! probably not what you think! actually, i will get back to this. first, let me diagram a simplified version of allocating memory in heap:
