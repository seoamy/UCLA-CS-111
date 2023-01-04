# NAME: Muthu Palaniappan, Amy Seo
# EMAIL: muthupal@g.ucla.edu, amyseo@ucla.edu
# ID: 005292734, 505328863

#!/usr/bin/env python3

import sys, csv, math
max_blocks = 0
original_block_bitmap = 0
original_inode_bitmap = 0
exit_code = 0

superblock_details = "SUPERBLOCK"
group_details = "GROUP"
free_block_details = "BFREE"
free_inode_details = "IFREE"
inode_details = "INODE"
direct_details = "DIRENT"
indirect_details = "INDIRECT"

def print_indirection(indirection_level):
  if indirection_level == 0:
      print("BLOCK", end = " ")
  elif indirection_level == 1:
      print("INDIRECT BLOCK", end = " ")
  elif indirection_level == 2:
      print("DOUBLE INDIRECT BLOCK", end = " ")
  elif indirection_level == 3:
      print("TRIPLE INDIRECT BLOCK", end = " ")

class Superblock:
  def __init__(self, superblock_row):
    self.max_blocks = int(superblock_row[1])
    self.max_inodes = int(superblock_row[2])
    self.block_size = int(superblock_row[3])
    self.inode_size = int(superblock_row[4])
    self.blocks_per_group = int(superblock_row[5])
    self.inodes_per_group = int(superblock_row[6])
    self.first_nonreserved_inode = int(superblock_row[7])
  
  def get_max_blocks(self):
    global max_blocks
    max_blocks = self.max_blocks
    return

class GroupDescriptor:
  def __init__(self, group_descriptor_row):
    self.group_num = int(group_descriptor_row[1])
    self.total_num_blocks_in_group = int(group_descriptor_row[2])
    self.total_num_inodes_in_group = int(group_descriptor_row[3])
    self.num_free_blocks = int(group_descriptor_row[4])
    self.num_free_inodes = int(group_descriptor_row[5])
    self.original_block_bitmap = group_descriptor_row[6]
    self.original_inode_bitmap = group_descriptor_row[7]
    self.block_num_of_first_inode_block = int(group_descriptor_row[8])

  def get_block_bitmap(self):
    global original_block_bitmap
    original_block_bitmap = self.original_block_bitmap
    return

  def get_inode_bitmap(self):
    global original_inode_bitmap
    original_inode_bitmap = self.original_inode_bitmap
    return

class Inode:
  def __init__(self, inode_row):
    self.inode_num = int(inode_row[1])
    self.file_type = inode_row[2]
    self.mode = int(inode_row[3]) & 0xFFF
    self.owner = int(inode_row[4])
    self.group = int(inode_row[5])
    self.link_count = int(inode_row[6])
    self.time_of_create = inode_row[7]
    self.time_of_mod = inode_row[8]
    self.time_of_access = inode_row[9]
    self.file_size = int(inode_row[10])
    self.num_blocks_taken_up = int(inode_row[11])
    self.blocks = []
    for i in range (12, len(inode_row)):
        self.blocks.append(int(inode_row[i]))

class DirEntry:
  def __init__(self, direct_row):
    self.parent_inode_num = int(direct_row[1])
    self.logical_byte_offset = int(direct_row[2])
    self.inode_num = int(direct_row[3])
    self.entry_length = int(direct_row[4])
    self.name_length = int(direct_row[5])
    self.name = direct_row[6]

class IndirectEntry:
  def __init__(self, indirect_row):
    self.inode_num_owner = int(indirect_row[1])
    self.indirect_level = int(indirect_row[2])
    self.logical_byte_offset = int(indirect_row[3])
    self.parent_block_num = int(indirect_row[4])
    self.child_block_num = int(indirect_row[5])

def Main():
    global exit_code
    if len(sys.argv) != 2:
        sys.stderr.write("Error in Usage: only enter one argument.\n")
        exit(1)
    filename = sys.argv[1]
    if filename[-4:] != ".csv":
        sys.stderr.write("Error in Usage: file is not a csv.\n")
        exit(1)
    file = csv.reader(open(filename), delimiter=',')

    # declare all the variables for this file system 
    this_superblock = None
    this_group_desc = None
    this_bfrees = [] # array of all free data blocks
    this_ifrees = [] # array of all free inodes
    this_inodes = [] # array of INODE OBJECTS
    this_dirents = [] # array of DIRENT OBJECTS
    this_indirects = [] # array of INDIRECT OBJECTS

    # parse through each line of csv
    for line in file:
        if superblock_details in line:
          this_superblock = Superblock(line)
          this_superblock.get_max_blocks()
        elif group_details in line:
          this_group_desc = GroupDescriptor(line)
          this_group_desc.get_block_bitmap()
          this_group_desc.get_inode_bitmap()
        elif free_block_details in line:
          block_num = int(line[1])
          this_bfrees.append(block_num)
        elif free_inode_details in line:
          inode_num = int(line[1])
          this_ifrees.append(inode_num)
        elif inode_details in line:
          this_inodes.append(Inode(line))
        elif direct_details in line:
          this_dirents.append(DirEntry(line))
        elif indirect_details in line:
          this_indirects.append(IndirectEntry(line))
        else:
          sys.stderr.write("Error in CSV: invalid line found.")
          exit(1)

    ###########################################################
    # Block consistency audits
    #
    lower_bound = int(math.ceil(this_group_desc.block_num_of_first_inode_block + this_group_desc.total_num_inodes_in_group * this_superblock.inode_size / this_superblock.block_size))
    upper_bound = this_superblock.max_blocks
    block_references = {} # [indirection level, inode number, offset]
    for item in this_inodes:
        if (item.file_type == 's' and item.file_size <= 60): # if inode not used
            continue
        for i, block in enumerate(item.blocks):
            # calculate offset
            indirection_level = 0 # 12 direct inodes have offset 0
            indirection_offset = 0
            if i == 12: # 13th inode is indirect with offset 12
                indirection_level = 1
                indirection_offset = 12
            elif i == 13: # 14th inode is double indirect with offset 12 + 256
                indirection_level = 2
                indirection_offset = 12 + 256
            elif i == 14: # 15th inode is triple indirect with offset 12 + 256 + (256 * 256)
                indirection_level = 3
                indirection_offset = 12 + 256 + (256 * 256)

            # add to block references
            if block not in block_references:
              block_references[block] = []
              block_references[block].append([indirection_level, item.inode_num, indirection_offset])
            elif block in block_references:
              block_references[block].append([indirection_level, item.inode_num, indirection_offset])

            # check for invalid block
            if block < 0 or block >= this_superblock.max_blocks: 
                print("INVALID", end = " ")
                print_indirection(indirection_level)
                print (str(block), end = " ")
                print ("IN INODE " + str(item.inode_num) + " AT OFFSET " + str(indirection_offset))
                exit_code = 2
                
            # check for reserved block
            # if block not in range(lower_bound, upper_bound):
            if block < lower_bound:
              if block == 0:
                continue
              print("RESERVED", end = " ")
              print_indirection(indirection_level)
              print(str(block) + " IN INODE " + str(item.inode_num) + " AT OFFSET " + str(indirection_offset))
              exit_code = 2
              
    # check all indirect block references
    for indirect in this_indirects:
      if (indirect.child_block_num not in block_references):
        block_references[indirect.child_block_num] = []
        block_references[indirect.child_block_num].append([indirect.indirect_level, indirect.inode_num_owner, indirect.logical_byte_offset])
      else:
        block_references[indirect.child_block_num].append([indirect.indirect_level, indirect.inode_num_owner, indirect.logical_byte_offset])

    # check unreferenced and allocated blocks
    for i in range(lower_bound, upper_bound):
      if (i not in this_bfrees) and (i not in block_references):
        print("UNREFERENCED BLOCK " + str(i))
        exit_code = 2
      if (i in this_bfrees) and (i in block_references):
        print("ALLOCATED BLOCK " + str(i)+ " ON FREELIST")
        exit_code = 2

    # check for duplicate blocks
    for i in range(lower_bound, upper_bound):
      if (i in block_references) and (len(block_references[i]) > 1):
        for ref in block_references[i]:
          print("DUPLICATE", end = " ")
          print_indirection(ref[0])
          print(str(i) + " IN INODE " + str(ref[1]) + " AT OFFSET " + str(ref[2]))
          exit_code = 2

    ########################################################### 
    ###########################################################
    # Inode allocation audits
    #
    allocated_inodes = []
    allocated_inodes_num = []
    unallocated_inodes = this_ifrees
    for item in this_inodes:
      allocated_inodes.append(item)
      allocated_inodes_num.append(item.inode_num)
      if ((item.file_type != '0') and  item.inode_num in this_ifrees):
        print("ALLOCATED INODE " + str(item.inode_num) + " ON FREELIST")
        unallocated_inodes.remove(item.inode_num)
        exit_code = 2
      elif ((item.file_type == '0') and item.inode_num not in this_ifrees):
        print("UNALLOCATED INODE " + str(item.inode_num) + " NOT ON FREELIST")
        allocated_inodes.remove(item)
        unallocated_inodes.append(item.inode_num)
        exit_code = 2

    for item in range(this_superblock.first_nonreserved_inode, this_superblock.max_inodes):
      if (item not in allocated_inodes_num):
        if (item not in this_ifrees):
          print("UNALLOCATED INODE " + str(item) + " NOT ON FREELIST")
          exit_code = 2
          unallocated_inodes.append(item)

    ##########################################################
    ###########################################################
    # Directory consistency audits
    #
    inode_ref_array = [0] * this_superblock.max_inodes
    inode_par_array = [0] * this_superblock.max_inodes
    for dirent in this_dirents:
      child_ino = dirent.inode_num
      child_name = dirent.name
      # check for invalid inode
      if (child_ino < 1 or child_ino > this_superblock.max_inodes):
        print("DIRECTORY INODE " + str(dirent.parent_inode_num) + " NAME " + str(child_name) + " INVALID INODE " + str(child_ino))
        exit_code = 2
        continue
      # check for unallocated 
      if (child_ino in unallocated_inodes):
        print("DIRECTORY INODE " + str(dirent.parent_inode_num) + " NAME " + str(child_name) + " UNALLOCATED INODE " + str(child_ino))
        exit_code = 2
        continue
      # increment link count
      inode_ref_array[child_ino] += 1
      # check for mismatch
      if (child_name == "'.'" and child_ino != dirent.parent_inode_num):
        print("DIRECTORY INODE " + str(dirent.parent_inode_num) + " NAME '.' LINK TO INODE " + str(child_ino) + "SHOULD BE " + str(dirent.parent_inode_num))
        exit_code = 2
        continue
      # increment link count & store parent inode
      if (child_name != "'.'" and child_name != "'..'"):
        inode_par_array[child_ino] = dirent.parent_inode_num
    
    for item in allocated_inodes:
      parent_ino = item.inode_num
      # report INCORRECT_LINK_COUNT
      if (inode_ref_array[parent_ino] != item.link_count):
        print("INODE " + str(parent_ino) + " HAS " + str(inode_ref_array[parent_ino]) + " LINKS BUT LINKCOUNT IS " + str(item.link_count))
        exit_code = 2

    for dirent in this_dirents:
      child_ino = dirent.inode_num
      child_name = dirent.name
      inode_par_array[2] = 2 # set root directory inode
      if child_name == "'..'" and child_ino != inode_par_array[dirent.parent_inode_num]:
        print("DIRECTORY INODE " + str(dirent.parent_inode_num) + " NAME '..' LINK TO INODE " + str(child_ino) + " SHOULD BE " + str(dirent.parent_inode_num))
        exit_code = 2

    ###########################################################
   
    exit(exit_code)

if __name__ == "__main__":
    Main()