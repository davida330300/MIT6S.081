# FS

## Large file

1. change NDIRECT from 12 to 11
2. add NDBIRECT = NDINIRECT*NDINIRECT
3. MAXFILE = NDIRECT + NDINIRECT + NDBIRECT

Modify bmap in kernel/fs.c and add two-level mapping in the case of bn < NDBINDIRECT. First, let bn subtract the NINDIRECT from the previous level. The first-level index mapped by DOUBLE INDIRECT BLOCK should be bn/NINDIRECT, and the second-level index should be bn%NINDIRECT.

Similar in itrunc, see how first level interact with second level, and add the interaction between second level and third level