# use after free(): how to exploit a flaw in glibc's heap management.

### what is heap? Please read about [heap vs. stack](https://www.geeksforgeeks.org/stack-vs-heap-memory-allocation/) if unfamiliar. 
your computer has storage (in this case, random access memory) which is always available for use by C executables. 
any given program starts at a location in that 'storage', and the stack specific to that program grows from that location onwards. 
the stack is fast to access, but it's only around 8 MB of space. in comparison, the heap is for dynamically allocated memory,
which can be much larger than 8 megabytes. as a downside, it's slightly slower to access (and you have to free the allocated memory manually)

> note: allocating on the stack is faster than to the heap (freeing also takes some time in heap malloc), but once a chunk of information is allocated on to the heap, it can be used with the same speed as the stack! realistically, this heavily depends on usage pattern because modern systems utilize caches (which allow for very fast access if all the data you use are placed close together in memory). for example, a linked list may have its node living at arbitrary spots on the heap, which will usually trash cache often, but a contiguous array will store its elements next to each other, and so iterating through elements is fast.


### dynamically allocated memory: memory allocated during runtime. 

`malloc(size_t size)` Allocates size bytes and returns that pointer to the programmer

`calloc(size_t, nmemb, size_t size)` Allocates nmemb*size bytes and zeros out the memory.

`free(void *ptr)` Frees the heap space pointed by ptr

what you need to know: dynamically allocating memory stores the 'information' into the heap instead of the stack (pictured below). to "free" this information stored in the heap, you use the `free()` function.

![imgonline-com-ua-Negative-4gopK6gZcTtQhf](https://user-images.githubusercontent.com/114739901/201462030-bd1b09a0-2615-4823-aeaf-5b010e4077ab.jpg)

> note: this diagram is outdated for modern use. there are multiple program images present in memory address space (main program + libraries) which have their own data, text, bss, etc. sections. the above heap and stack model diagram above is managed by brk (and sbrk) system calls, called program break. it extends beyond the bss section of the program (where all initialised globals live) and grows toward the stack. On modern systems, however, the location of the stack & heap is completely arbitrary, and they're usually in disconnected areas. This is more true on 64-bit systems, where big addresses  (at least of 48-bits, sometimes up to 52-bits of meaningful data) allow for huge gaps between memory segments. The memory layout is now usually randomized, through mechanism called ASLR (address-space layout randomization) to prevent some security issues concerning an attacker being able to jump to arbitrary location, given they know the address to target, with ASLR, they can't. However, that's only damage control and a proper system must ensure this kind of situation can't happen in the first place.


### what does it mean to `free()` allocated memory?
haha! probably not what you think! actually, i will get back to this. first, let me diagram a simplified version of allocating memory in heap:

![imgonline-com-ua-Negative-nZlejKCn83UXN](https://user-images.githubusercontent.com/114739901/201462075-3fa59d9f-0164-440e-bd15-5cb12e2a6f49.jpg)

In this example, i've allocated memory in the heap, and the address to each respective chunk of memory is stored in `x` and `y`. if i use `free(x)` or `free(y)`, it does not _remove_ the allocated memory from the heap (this is the counterintuitive part)! a computer can either store or retrieve a value, not delete (although, storing a value can replace the previous value, inherently destroying the previous contents of the memory cell. this is still different than removing a chunk of memory of the heap). Instead of _removing_ the allocated chunk of memory from the heap, the heap manager will put the same chunk of memory into a cache and label it as "available for usage". This is scuffed! It is also optimal! the reusage and recycling of memory is what helps your computer not explode over time.

> note: `x` and `y` are pointers to an arbitrary location (with a unique address) of 0x10 bytes of allocated memory in the heap

> note: because information stored in the heap is stored in the RAM (usually volatile memory), everything in RAM will be lost when the computer is turned off. this is a way of "deleting" memory chunks stored in the heap through technicality. 

each of these new locations on the heap has a unique address. dumbed down, you can imagine computer memory as a sequence of storage compartments called `memory cells`. each memory cell has a unique address (indicating its relative position in memory). most computers have millions of individual memory cells (each with their own unique address), and the information stored inside each cell is called the `contents` of the cell. **every memory cell has contents**, but normally we only know the information of the cells whose contents we replace manually (variable assignment, etc.) or are for example replaced by an automated process during compilation (stored program concept). 

when you call `free(x)`, it does not _remove_ the memory chunk (with a unique address) from the heap. **instead, it stores the memory chunk in a cache (specific cache bin depends on byte size of chunk) and labels the chunk as 'available for usage', allowing for the same chunk and address to be used by another memory allocation process (pointers, etc.)**.

> note: the algorithm for `free()` first checks to see if the chunk before _or_ after the freed chunk is also freed (in cache). if this is the case, the chunk will merge with the other freed chunk to create a bigger freed chunk. if there are no neighboring freed chunks, the chunk actively affected by `free()` will be marked as "available for usage" and placed in an appropriate bin/cache.
