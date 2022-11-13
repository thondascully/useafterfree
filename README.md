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

![imgonline-com-ua-Negative-4gopK6gZcTtQhf](https://user-images.githubusercontent.com/114739901/201462030-bd1b09a0-2615-4823-aeaf-5b010e4077ab.jpg)

> note: this diagram is outdated for modern use. there are multiple program images present in memory address space (main program + libraries) which have their own data, text, bss, etc. sections. the above heap and stack model diagram above is managed by brk (and sbrk) system calls, called program break.  it extends beyond the bss section of the program (where all unitialised globals live) and grows toward the stack. On modern systems, however, the location of the stack & heap is completly arbitrary, and they're usually in disconnected areas. This is more true on 64-bit systems, where big addresses  (at least of 48-bits, sometimes up to 52-bits of meaningful data) allow for huge gaps between memory segments. The memory layout is now usually randomised, through mechanism called ASLR (address-space layout randomisation) to prevent some security issues concerning an attacker being able to jump to arbitrary location, given they know the address to target, with ASLR, they can't. However, that's only damage control and a proper system must ensure this kind of situation can't happen in the first place.


### what does it mean to `free()` allocated memory?
haha! probably not what you think! actually, i will get back to this. first, let me diagram a simplified version of allocating memory in heap:

![imgonline-com-ua-Negative-nZlejKCn83UXN](https://user-images.githubusercontent.com/114739901/201462075-3fa59d9f-0164-440e-bd15-5cb12e2a6f49.jpg)

In this example, i've allocated memory in the heap for `x` and `y`. if i use `free(x)` or `free(y)`, it does not remove the allocated memory from the heap (this is the counterintuitive part)! Instead of 'deleting' the allocated chunk of memory from the heap, the heap manager will instead put the same chunk of memory into a cache and label it as "available for usage". This is scuffed! It is also optimal! the reusage and recycling of memory is what helps your computer not explode over time.
