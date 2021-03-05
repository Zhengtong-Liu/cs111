# NAME:		Bryan Tang, Zhengtong Liu
# EMAIL: 	tangtang1228@ucla.edu, ericliu2023@g.ucla.edu
# ID:    	605318712, 505375562


#       Boot block |    Block Group 0   | Block Group 1 | ... | Block Group N
#                  /                    \       
#                 /                      \
# superblock |GroupDescriptors |BlockBitmap |InodeBitmap |InodeTable |DataBlock
import sys

isConsistent = True

def directory_consistency_audits(inode_dir_info, inode_par_array, inode_link_counts, inode_ref_counts, ifree):
    global isConsistent

    # unallocated
    for child_inode in inode_dir_info:
        dir_name = inode_dir_info[child_inode][0]
        parent_inode = inode_dir_info[child_inode][1]
        if (child_inode in ifree) and (child_inode in inode_par_array):
            print(f'DIRECTORY INODE {str(parent_inode)} NAME {str(dir_name)} UNALLOCATED INODE {str(child_inode)}')
            isConsistent = False

    # parent mismatch
    for child_inode in inode_par_array:
        dir_name = inode_dir_info[child_inode][0]
        parent_inode = inode_par_array[child_inode]

        if (dir_name == "'..'") and child_inode != parent_inode:
            print(f'DIRECTORY INODE {str(parent_inode)} NAME {str(dir_name)} LINK TO INODE {str(child_inode)} SHOULD BE {str(parent_inode)}')
            isConsistent = False

    # link counts mismatch
    for child_inode in inode_link_counts:

        link_counts = inode_link_counts[child_inode]

        actual_link = 0
        if child_inode in inode_ref_counts:
            actual_link = inode_ref_counts[child_inode]
        
        if actual_link != link_counts:
            print(f'INODE {str(child_inode)} HAS {str(actual_link)} LINKS BUT LINKCOUNT IS {str(link_counts)}')
            isConsistent = False

def inode_allocation_audits(inode_link_counts, inodes_count, ifree, first_inode):
    global isConsistent

    # check for allocation of inodes
    for k in range(inodes_count):
        index = k + 1
        if (index in ifree) and (index in inode_link_counts):
            print(f'ALLOCATED INODE {str(index)} ON FREELIST')
            isConsistent = False
        elif (index not in ifree) and (index not in inode_link_counts) and (index >= first_inode or index == 2):
            print(f'UNALLOCATED INODE {str(index)} NOT ON FREELIST')
            isConsistent = False

def block_consistency_audits(blockDict, blocks_count, bfree, non_reserved_block_start):
    global isConsistent

    # check allocation and reference
    for k in range(blocks_count-1):
        index = k + 1
        if (index in blockDict) and (index in bfree):
            print(f'ALLOCATED BLOCK {str(index)} ON FREELIST')
            isConsistent = False
        elif (index not in blockDict) and (index not in bfree) and (index >= non_reserved_block_start):
            print(f'UNREFERENCED BLOCK {str(index)}')
            isConsistent = False

    # check duplication
    for blk in blockDict:
        if len(blockDict[blk]) > 1:
            for piece in blockDict[blk]:
                inode_num = int(piece[0])
                level = int(piece[1])
                if level == 0:
                    blockType = 'BLOCK'
                    offset = 0
                if level == 1:
                    blockType = 'INDIRECT BLOCK'
                    offset = 12
                elif level == 2:
                    blockType = 'DOUBLE INDIRECT BLOCK'
                    offset = 12 + 256
                elif level == 3:
                    blockType = 'TRIPLE INDIRECT BLOCK'
                    offset = 12 + 256 + 256**2
                print(f'DUPLICATE {blockType} {str(blk)} IN INODE {str(inode_num)} AT OFFSET {str(offset)}')
                isConsistent = False

def main():
    global isConsistent
    # check the number of arguments; if apart from the name of executable, the arugment is not
    # one, then the argument(s) is/are invalid
    if len(sys.argv) != 2:
        sys.stderr.write("Invalid Argument(s)\n")
        exit(1)
    # attemp to open the input file
    try:
        file = open(sys.argv[1], "r")
    except:
        sys.stderr.write("cannot open the file\n")
        exit(1)

    bfree = []
    ifree = []

    numOfInodes = 0
    blocks_count = 0
    inodes_count = 0
    block_size = 0
    inode_size = 0

    # the start of valid blocks and first valid inode in inode table
    non_reserved_block_start = 0
    first_inode = 0

    inode_dir_info = {} # key: inode_num; value: [dir_name, parent_inode_num]
    inode_link_counts = {} # key: inode_num; value: link counts (theoretical)
    inode_ref_counts = {} # key: inode_num; value: reference counts (actual)
    inode_par_array = {} # key: child_inode_num; value: parent_inode_num
    blockDict = {} # key: block number; value: [inode_num, level]

    lines = file.readlines()

    # parse the types and do checks that do not need the whole bitmap for blocks or inodes
    for line in lines:
        fields = line.split(",")
        Type = fields[0]

        if Type == 'SUPERBLOCK':
            # useful info from super block: block boundary, inode boundary, block size, inode size
            # and the position of first inode
            blocks_count = int(fields[1])
            inodes_count = int(fields[2])
            block_size = int(fields[3])
            inode_size = int(fields[4])
            first_inode = int(fields[7])
        elif Type == 'GROUP':
            # useful info from group: number of inodes in this group, the start of inode table
            numOfInodes = int(fields[3])
            bg_inode_table = int(fields[8])
            # determine the first valid datablock from the start of inode table and the size of inode table
            non_reserved_block_start = bg_inode_table + inode_size * numOfInodes / block_size
        # keep track of the free lists
        elif Type == 'BFREE':
            bfree.append(int(fields[1]))
        elif Type == 'IFREE':
            ifree.append(int(fields[1]))
        # do some checking
        elif Type == 'INODE':
            inodeNum = int(fields[1])
            fileType = fields[2]
            # keep track of the theoretical link counts
            inode_link_counts[inodeNum] = int(fields[6])

            if fileType != 's':
                for i in range(12,27):
                    blockNum = int(fields[i])

                    if blockNum == 0: #skip if 0
                        continue
                    # match the position with the right level, offset and block type
                    if i < 24:
                        blockType = 'BLOCK'
                        offset = 0
                        level = 0
                    elif i == 24:
                        blockType = 'INDIRECT BLOCK'
                        offset = 12
                        level = 1
                    elif i == 25:
                        blockType = 'DOUBLE INDIRECT BLOCK'
                        offset = 12 + 256
                        level = 2
                    elif i == 26:
                        blockType = 'TRIPLE INDIRECT BLOCK'
                        offset = 12 + 256 + 256**2
                        level = 3
                    # invalid
                    if blockNum < 0 or blockNum > blocks_count:
                        print(f'INVALID {blockType} {str(blockNum)} IN INODE {str(inodeNum)} AT OFFSET {str(offset)}')
                        isConsistent = False
                    # reserved
                    if 0 < blockNum < non_reserved_block_start:
                        print(f'RESERVED {blockType} {str(blockNum)} IN INODE {str(inodeNum)} AT OFFSET {str(offset)}')
                        isConsistent = False
                    # duplicated
                    elif blockNum in blockDict:
                        blockDict[blockNum].append([inodeNum, level])
                    # not in blockDict
                    else: 
                        blockDict[blockNum] = [[inodeNum, level]]

        elif Type == 'INDIRECT':
            inodeNum = int(fields[1])
            level = int(fields[2])
            blockNum = int(fields[5])
            # get the right block type and offset from the level
            if level == 1:
                blockType = 'INDIRECT BLOCK'
                offset = 12
            elif level == 2:
                blockType = 'DOUBLE INDIRECT BLOCK'
                offset = 12 + 256
            elif level == 3:
                blockType = 'TRIPLE INDIRECT BLOCK'
                offset = 12 + 256 + 256**2
            # invalid 
            if blockNum < 0 or blockNum > blocks_count:
                print(f'INVALID {blockType} {str(blockNum)} IN INODE {str(inodeNum)} AT OFFSET {str(offset)}')
                isConsistent = False
            # reserved
            if blockNum < non_reserved_block_start:
                print(f'RESERVED {blockType} {str(blockNum)} IN INODE {str(inodeNum)} AT OFFSET {str(offset)}')
                isConsistent = False
            # duplicated
            elif blockNum in blockDict:
                blockDict[blockNum].append([inodeNum, level])
            else: # not in blockDict
                blockDict[blockNum] = [[inodeNum, level]]

        elif Type == 'DIRENT':
            parent_inode_num = int(fields[1])
            inode_num = int(fields[3])
            dir_name = fields[6]
            # slice off the '\n' character
            dir_name = dir_name[:-1]
            # keep track of reference counts to compare with the link counts
            if inode_num in inode_ref_counts:
                inode_ref_counts[inode_num] += 1
            else:
                inode_ref_counts[inode_num] = 1
            # useful when checking allocation and parent mismatch afterwards
            inode_dir_info[inode_num] = [str(dir_name), int(parent_inode_num)]
        
            # invalid
            if inode_num < 1 or inode_num > inodes_count:
                print(f'DIRECTORY INODE {str(parent_inode_num)} NAME {str(dir_name)} INVALID INODE {str(inode_num)}')
                isConsistent = False
            # current mismatch
            if str(dir_name) == "'.'" and parent_inode_num != inode_num:
                print(f'DIRECTORY INODE {str(parent_inode_num)} NAME {str(dir_name)} LINK TO {str(inode_num)} SHOULD BE {str(parent_inode_num)}')
                isConsistent = False
            # keep track of the parent of an inode for parent mismatch afterwards
            if str(dir_name) != "'.'" and str(dir_name) != "'..'":
                inode_par_array[inode_num] = parent_inode_num
        
    # check for unallocation, parent mismatch and link counts mismatch
    directory_consistency_audits(inode_dir_info, inode_par_array, inode_link_counts, inode_ref_counts, ifree)
    # check for allocation of inodes
    inode_allocation_audits(inode_link_counts, inodes_count, ifree, first_inode)
    # check for allocation, duplication, and unreference of blocks
    block_consistency_audits(blockDict, blocks_count, bfree, non_reserved_block_start)

    # exit with the proper exit code
    if isConsistent:
        exit(0)
    else:
        exit(2)


if __name__ == '__main__':
    main()
