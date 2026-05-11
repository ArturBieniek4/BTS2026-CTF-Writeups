The flag exists in memory at address `0x40023000`.
Reading it directly with `8) peek somewhere` option faults, because the page is mapped kernel-only.
We can however exploit the fact that the page table is user-writeable, together with a flawed pony index check to change that:
- We pick a pony number `0xF000000000000111`
    - This passes the `if (index > 255)` check because the value has the sign bit set and is treated as signed-negative by the comparison
    - The low 64 bits become `0x1110` after multiplying by 16
    - After adding the `.barn` base address, we get `0x40021110`
- We change pony's magic number to `0x0060000040023743`
    - This directly writes to address `0x40021118`, which is the PTE for `.flag`
    - The original value there is `0x0060000040023703`
    - We change just one bit to grant access from usermode
- We use option `9) sparkle dust`
    - This flushes the TLB, so the edited page-table permissions take effect
- We read the flag using option `8) peek somewhere`
    - This can read anything from the memory usermode has access to, so we can now use it
    - It reads 8 bytes, so we have to use it 5 times on subsequent addresses to get the full flag text
