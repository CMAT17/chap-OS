#include "file_sys_module.h"
#include "multiboot.h"
#include "lib.h"
#include "system_call.h"


static uint32_t file_sys_start;
static boot_block_t* boot_block_ptr;
static uint32_t inode_start;
static uint32_t data_block_start;

/* file_sys_init
*       Obtain the addresses of the start of the filesystem, the boot block, start of the inodes, and start of the
*       data blocks
* INPUT: file_sys_module - address of the filesystem module
* OUTPUT: none
* RETURN VALUE: none
*/
void file_sys_init(module_t* file_sys_module)
{
    
    file_sys_start = file_sys_module->mod_start;
    printf("%d\n",file_sys_start);
    //First inode address is the second 4kB block
    inode_start = file_sys_start + DATA_BLOCK_SIZE;
    //boot block is the first 4kB block
    boot_block_ptr = (boot_block_t *)file_sys_start;
    //Data block address starts after the inode blocks
    data_block_start = file_sys_start + (1+boot_block_ptr->num_inodes)*DATA_BLOCK_SIZE;
}

/* file_sys_open
*       Open the file system. Should never be called.
* INPUT: ignored
* OUTPUT: none
* RETURN VALUE: 0
*/
int32_t file_sys_open(const uint8_t * filename)
{
    return 0;
}

/* file_sys_close
*       Close the file system. Should never be called.
* INPUT: ignored
* OUTPUT: none
* RETURN VALUE: 0
*/
int32_t file_sys_close(int32_t fd)
{
    return 0;
}
/* file_sys_write
*       Write into the file system. Should never be called.
* INPUT: ignored
* OUTPUT: none
* RETURN VALUE: -1 (should always fail)
*/
int32_t file_sys_write(int32_t fd, const void* buf, int32_t nbytes)
{
    return -1;
}

/*
 * read_dentry_by_name
 *         Searches filesystem for given filename and if found copies
 *         file information into dentry
 * INPUT: fname - filename to be searched for in filesystem
 *        dentry - filled in with file name, file type, and inode number
 * OUTPUT: none
 * RETURN VALUE: 0 on success, -1 on failure
 */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry)
{
    int file_not_found = FILE_NOT_FOUND;
    int i = 0;

    //iterate through directory entries until fname matches the dentry filename
    while(file_not_found == 1 && i < MAX_DENTRY_NUM)
    {
        if(strncmp((int8_t*)fname,(int8_t*) boot_block_ptr->dir_entries[i].file_name, FILE_NAME_SIZE) == 0){
            file_not_found = FILE_FOUND;
            //copy file name
            strncpy((int8_t*)dentry->file_name, (int8_t*)fname, FILE_NAME_SIZE);

            //copy file type
            dentry->file_type = boot_block_ptr->dir_entries[i].file_type;

            //copy inode number
            dentry->inode_num = boot_block_ptr->dir_entries[i].inode_num;
        }
        i++;
    }

    //return -1 if file is nonexistent 
    return (file_not_found == FILE_NOT_FOUND ? -1 : 0); 
}

/*
 * read_dentry_by_index
 * INPUT: index - index of directory entry
 *        dentry - filled in with file name, file type, and inode number
 * OUTPUT: none
 * RETURN VALUE: 0 on success, -1 on failure
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry)
{
    //check if invalid index
    if(index>=MAX_DENTRY_NUM)
    {
        return -1;
    }

    //check if filename is actually valid and not empt
    if((int8_t)boot_block_ptr->dir_entries[index].file_name[0]=='\0')
    {
        return -1;
    }
    ///fills in dentry with file name, type, and inode number
    strncpy((int8_t*)dentry->file_name, (int8_t*)boot_block_ptr->dir_entries[index].file_name, FILE_NAME_SIZE);

    //populate the dentry file_type and inode_num fields for future usage
    dentry->file_type = boot_block_ptr->dir_entries[index].file_type;
    dentry->inode_num = boot_block_ptr->dir_entries[index].inode_num;

    return 0;
}

/*
 * read_data
 * INPUT: inode - index node
 *        offset - starting position in file
 *        buf - data stored in this buffer
 *        length - read up to this number of bytes
 * OUTPUT: none
 * RETURN VALUE: number of bytes read, 0 indicatees end of file, -1 on failure
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
    int i;
    uint32_t cur_block_index;
    uint32_t loc_cur_block_index;
    uint32_t cur_data_block;
    uint8_t* read_data_addr;

    //get the particular inode pointer
    inode_t* inode_data_ptr = (inode_t *) (inode_start + (inode)*DATA_BLOCK_SIZE);
    
    //check if given inode number is in range
    if(inode >= boot_block_ptr->num_inodes)
    {
        return -1;
    }
    //no bytes will be read because offset is greater than the length
    if(offset >= inode_data_ptr->length)
    {
        return 0;
    }
    
    //obtain the current data block number being read from
    cur_block_index = offset/DATA_BLOCK_SIZE;

    //obtain the starting location within that data block
    loc_cur_block_index = offset % DATA_BLOCK_SIZE;

    //obtain the address of the desired data block
    cur_data_block = data_block_start+(inode_data_ptr->data_block[cur_block_index]) * DATA_BLOCK_SIZE;

    i = 0;
    while(i < length && i+offset <= inode_data_ptr->length)
    {

        // check if the location within the data block is at the very end, if so, then go to the next block
        if(loc_cur_block_index == DATA_BLOCK_SIZE)
        {
            cur_block_index++;
            loc_cur_block_index = 0;
            cur_data_block = data_block_start+(inode_data_ptr->data_block[cur_block_index]) * DATA_BLOCK_SIZE;
        }

        //Because we don't necessarily know whether or not all data blocks for a praitcular file are contiguous, we must tack on each byte individually
        //find the address of the byte we want to copy over
        read_data_addr = (uint8_t*)(cur_data_block+loc_cur_block_index);
        buf[i] = *read_data_addr;
        
        //increment number of bytes read thus far, and move the location within the block up
        i++;
        loc_cur_block_index++;
    }
    return i;
}

/* file_open
*       Open a file. Does nothing, as all relevant information is handled in the open syscall
* INPUT: ignored
* OUTPUT: none
* RETURN VALUE: 0
*/
int32_t file_open(const uint8_t * filename)
{
    return 0;
}

/* file_close
*       Close a file. Does nothing, as all relevant information is handled in the close syscall
* INPUT: ignored
* OUTPUT: none
* RETURN VALUE: 0
*/
int32_t file_close(int32_t fd)
{
    return 0;
}

/* file_write
*       Write into a file. Should automatically fail, as it is a read-only file system
* INPUT: ignored
* OUTPUT: none
* RETURN VALUE: -1 (should always fail)
*/
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes)
{
    return -1;
}

/*
 * file_read
 *        Read in desired data from a file into a provided buffer
 * INPUT: fd - file descriptor in array of pcb
 *        buf - data stored in this buffer
 *        nbytes - read this number of bytes
 * OUTPUT: none
 * RETURN VALUE: number of bytes read
 */
int32_t file_read(int32_t fd, void* buf, int32_t nbytes)
{
    uint32_t offset, inode_num;
    int32_t nbytes_read;
    pcb_t* cur_pcb;
    cur_pcb = get_pcb_ptr();
    
    //obtain the inode number and offset from the file descriptor
    inode_num = cur_pcb->f_descs[fd].inode;
    offset = cur_pcb->f_descs[fd].file_pos;

    //read in the data into buffer provided
    nbytes_read = read_data(inode_num, offset, (uint8_t *) buf, nbytes);

    //increment the offset by however many bytes were read
    cur_pcb->f_descs[fd].file_pos += nbytes_read;
    return nbytes_read;
}

/* dir_open
*       Open a directory. Does nothing, as all relevant information is handled in the open syscall
* INPUT: ignored
* OUTPUT: none
* RETURN VALUE: 0
*/
int32_t dir_open(const uint8_t * filename)
{
    return 0;
}

/* dir_close
*       Close a directory. Does nothing, as all relevant information is handled in the close syscall
* INPUT: ignored
* OUTPUT: none
* RETURN VALUE: 0
*/
int32_t dir_close(int32_t fd)
{
    return 0;
}

/* dir_write
*       Write into a directory. Should automatically fail, as it is a read-only file system
* INPUT: ignored
* OUTPUT: none
* RETURN VALUE: -1 (should always fail)
*/
int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes)
{
    return -1;
}


/*
 * dir_read
 *        Read in the file name at the current file position index
 * INPUT: fd - file descriptor in array of pcb
 *        buf - data stored in this buffer
 *        nbytes - read this number of bytes
 * OUTPUT: none
 * RETURN VALUE: number of bytes read, 0 indicatees end of file
 */
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes)
{
    dentry_t dir_entry;
    
    pcb_t* cur_pcb =  get_pcb_ptr();
    
    int32_t test = read_dentry_by_index(cur_pcb->f_descs[fd].file_pos, &dir_entry);
    
    //Check for valid dentry, i.e. file is existent
    if(test == 0)
    {
        //dentry can be read by index
        //copy the individual file_name into the buffer
        memcpy(buf, (void*)dir_entry.file_name, FILE_NAME_SIZE);

        //increment the file position index to the next file
        cur_pcb->f_descs[fd].file_pos++;
        return FILE_NAME_SIZE;
    }
    else
    {
        //dentry is not valid, reset file position index
        cur_pcb->f_descs[fd].file_pos = 0;
        return 0;
    }
}

/*
 * get_file_size
 * INPUT: inode_num - number of inode
 * OUTPUT: none
 * RETURN VALUE: length in bytes of file
 */
uint32_t get_file_size(uint32_t inode_num)
{
    inode_t * f_inode;

    //obtain the address of the desired inode
    f_inode = (inode_t*)(inode_start + inode_num*DATA_BLOCK_SIZE);
    return f_inode->length;
}





