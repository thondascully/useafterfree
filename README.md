# use after free(): how to exploit a flaw in glibc's heap management.

### what is heap? Please read about [heap vs. stack](https://www.geeksforgeeks.org/stack-vs-heap-memory-allocation/) if unfamiliar. 
your computer has storage (in this case, random access memory) which is always available for use by C executables. 
any given program starts at a location in that 'storage', and the stack specific to that program grows from that location onwards. 
the stack is fast to access, but it's only around 8 MB of space. in comparison, the heap is for dynamically allocated memory,
which can be much larger than 8 megabytes. as a downside, it's slightly slower to access (and you have to free the allocated memory manually)

> note: allocating on the stack is faster than on the heap (freeing also takes some time in heap malloc), but once a chunk of information is allocated on the heap, it can be used with the same speed as information on the stack. realistically, this heavily depends on usage pattern because modern systems utilize caches (which allow for very fast access if all the data you use are placed close together in memory). for example, a linked list may have its node living at arbitrary spots on the heap, which will usually trash cache often, but a contiguous array will store its elements next to each other, and so iterating through elements is fast.


### dynamically allocated memory: memory allocated during runtime. 

`malloc(size_t size)` Allocates size bytes and returns pointer

`calloc(size_t, nmemb, size_t size)` Allocates nmemb*size bytes and zeros out the memory.

`free(void *ptr)` Frees the memory chunk indicated by pointer

what you need to know: dynamically allocating memory stores the 'information' into the heap instead of the stack (pictured below). to "free" this information stored in the heap, you use the `free()` function.

![imgonline-com-ua-Negative-mJTz68S5phDtzG](https://user-images.githubusercontent.com/114739901/201523164-53394c06-0254-46af-808f-1776fdb05ecb.jpg)

> note: this diagram is outdated for modern use. there are multiple program images present in memory address space (main program + libraries) which have their own data, text, bss, etc. sections. the above heap and stack model diagram above is managed by brk (and sbrk) system calls, called program break. it extends beyond the bss section of the program (where all initialised globals live) and grows toward the stack. On modern systems, however, the location of the stack & heap is completely arbitrary, and they're usually in disconnected areas. This is more true on 64-bit systems, where big addresses  (at least of 48-bits, sometimes up to 52-bits of meaningful data) allow for huge gaps between memory segments. The memory layout is now usually randomized, through mechanism called ASLR (address-space layout randomization) to prevent some security issues concerning an attacker being able to jump to arbitrary location given they know the address to target. [Pretty much, with ASLR, they can't](https://securitylab.github.com/research/last-orders-at-the-house-of-force/). However, that's only damage control and a proper system must ensure this kind of situation can't happen in the first place.


### what does it mean to `free()` allocated memory?
haha! probably not what you think! actually, i will get back to this. first, let me diagram a simplified version of allocating memory in heap:

![imgonline-com-ua-Negative-yODijXsRuAkkRCaE](https://user-images.githubusercontent.com/114739901/201523181-2ccbe8e1-1b43-48c4-9d84-798f6aa190b8.jpg)

In this example, i've allocated memory in the heap, and the address to each respective chunk of memory is stored in `x` and `y`. if i use `free(x)` or `free(y)`, it does not _remove_ the allocated memory from the heap (this is the counterintuitive part)! a computer can either store or retrieve a value, not delete (although, storing a value can replace the previous value, inherently destroying the previous contents of the memory cell. this is still different than removing a chunk of memory of the heap). Instead of _removing_ the allocated chunk of memory from the heap, the heap manager will put the same chunk of memory into a cache and label it as "available for usage". This is scuffed! It is also optimal! the reusage and recycling of memory is what helps your computer not explode over time.

> note: `x` and `y` are pointers to an arbitrary location (with a unique address) of 0x10 bytes of allocated memory in the heap

> note: because information stored in the heap is stored in the RAM (usually volatile memory), everything in RAM will be lost when the computer is turned off. this is a way of "deleting" memory chunks stored in the heap through technicality. 

each of these new locations on the heap has a unique address. dumbed down, you can imagine computer memory as a sequence of storage compartments called `memory cells`. each memory cell has a unique address (indicating its relative position in memory). most computers have millions of individual memory cells (each with their own unique address), and the information stored inside each cell is called the `contents` of the cell. **every memory cell has contents**, but normally we only know the information of the cells whose contents we replace manually (variable assignment, etc.) or are for example replaced by an automated process during compilation (stored program concept). 

when you call `free(x)`, it does not _remove_ the memory chunk (with a unique address) from the heap. **instead, it stores the memory chunk in a cache (specific cache bin depends on byte size of chunk) and labels the chunk as 'available for usage', allowing for the same chunk and address to be used by another memory allocation process (pointers, etc.)**.

> note: the algorithm for `free()` first checks to see if the chunk before _or_ after the freed chunk is also freed (exists in cache). if this is the case, the chunk will merge with the other freed chunk to create a bigger freed chunk. if there are no neighboring freed chunks, the `free()` algorithm will then mark the freed chunk as "available for usage" and place it in its appropriate bin/cache based on its size.

> note: different types of caches/bins exist. [tcache](https://sourceware.org/glibc/wiki/MallocInternals#Thread_Local_Cache_.28tcache.29) and [fast bins](https://sourceware.org/glibc/wiki/MallocInternals#Arenas_and_Heaps) are considered "in use", so they will not merge with adjacent freed chunks. 

### a refresher on tcache: 
processes run with one or more threads at the same time. all threads in a specific process have access to the same heap, but each individual thread has access to its own stack (cool). **all threads have access to the same heap**. this can be an issue if one thread is writing on the heap while another thread is reading from the same address at the same time. a simple solution to this is to use a [lock](https://en.wikipedia.org/wiki/Lock_(computer_science)), which is pretty much an activity boolean check before writing. unfortunately, this means there is a lot of stall time spent waiting for other operations to finish. while the stall time is minimal, the heap is always in use by all threads, so it can quickly add up to a slower program. 

tcache (per-thread cache) is designed to use [per-thread arenas](https://siddhesh.in/posts/malloc-per-thread-arenas-in-glibc.html), which is the manager's solution to the problem described above. in this context, an arena is a big blob of neighboring memory chunks. for example, the heap is an arena (typically referred to as the main arena). the 'per-thread arena' model is based on the creation of a new arena when memory is allocated for the first time within a specific thread. **therefore, one can expect no collision among threads writing on the heap because each thread is working within their own arena.**

each thread has 64 different [singly-linked-list](https://www.geeksforgeeks.org/what-is-linked-list/) tcache bins. each bin can only hold [seven](https://sourceware.org/git/?p=glibc.git;a=blob;f=malloc/malloc.c;h=2527e2504761744df2bdb1abdc02d936ff907ad2;hb=d5c3fafc4307c9b7a4c7d5cb381fcdbfad340bcc#l323) different memory chunks. if a bin of a specific byte size runs out of space (or if chunk size exceeds max tcache bin size), the manager will resort to the normal heap lock process with the fast bin (or small bin, large bin, unsorted bin; depends on size) recycling process.

![imgonline-com-ua-Negative-YUDczypxbprTp](https://user-images.githubusercontent.com/114739901/201523182-01b21237-b745-447c-bf03-58bea5a3908c.jpg)

if the memory chunk is of size 16 bytes, it will get cached in the bytes 0..24 bin (24). likewise, if 24 bytes, then bytes 0..24 bin (24). in comparison, a 32 byte chunk would get stored in the bytes 24..40 bin (40). Only [seven](https://sourceware.org/git/?p=glibc.git;a=blob;f=malloc/malloc.c;h=2527e2504761744df2bdb1abdc02d936ff907ad2;hb=d5c3fafc4307c9b7a4c7d5cb381fcdbfad340bcc#l323) different chunks can be stored within each sized bin.

### what happens when you `free()` (revisited):

in the context of the `x` pointer created above, a call of `free(x)` stores the memory chunk in a cache (specific cache bin depends on byte size of chunk) and labels the chunk as 'available for usage', allowing for the same chunk and address to be used by another memory allocation process.

![imgonline-com-ua-Negative-RE39sSX6e9f9yyp](https://user-images.githubusercontent.com/114739901/201523166-51523bed-ba5d-4463-b764-2f73fe7bdcd5.jpg)

> note: the `memory chunk stored in cache after being freed` is now labeled as `available for usage` by the computer.

one of the key things for this exploit is realizing that `x` still points to the same memory chunk address in the heap despite `x` being freed. therefore, **when `y = malloc(0x10);` is called and new memory is allocated of the same byte size as `x`, the manager will check the tcache to see if a chunk of that size exists (which is true now) and then 'assigns' that chunk to the new pointer of the allocated memory (`y` in this case).**

**because `x` still points to the same address that `y`'s chunk exists at, they both are pointing to the same memory chunk.**
